#include "scheduler.h"

Scheduler::Scheduler(QWidget *parent,DAO *dao)
    : QWidget(parent),dao(dao)
{
    // 动态分配套接字空间
    tcpServer=new QTcpServer(this);
}

Scheduler::~Scheduler()
{

}

void Scheduler::init(int mode,int defaultTemp,int defaultFanSpeed,float feeRate,float fan[3],int tempLowLimit,int tempHighLimit)
{
    this->mode=mode;
    this->defaultTemp=defaultTemp;
    this->defaultFanSpeed=defaultFanSpeed;
    this->feeRate=feeRate;
    this->fan[0]=fan[0];
    this->fan[1]=fan[1];
    this->fan[2]=fan[2];
    this->tempLowLimit=tempLowLimit;
    this->tempHighLimit=tempHighLimit;

    serverQueue=new ServerQueue(defaultTemp,defaultFanSpeed,feeRate,this);
    waitQueue=new WaitQueue(defaultTemp,defaultFanSpeed,feeRate,this);

    for (int i=0;i<3;i++)
    {
        Server *server=new Server(i,mode,defaultTemp,defaultFanSpeed,feeRate,fan,this);
        serverList.append(server);
    }

    for (int i=0;i<3;i++)
    {
        // 服务完成
        connect(serverList[i],&Server::signalServeOver,[=]()
        {
            sync(serverList[i]->roomId);
            RequestOver(serverList[i]->roomId);
        });

        // 温度变化，发送温度费用心跳包
        connect(serverList[i], &Server::signalSendInfo,[=](){
            // 刷新当前温度，总费用，服务时间
            allRoomMap[serverList[i]->roomId]->currentTemp=QString::number(serverList[i]->currentTemp, 'f', 2).toFloat();
            allRoomMap[serverList[i]->roomId]->fee=QString::number(serverList[i]->fee, 'f', 2).toFloat();
            allRoomMap[serverList[i]->roomId]->serverTime=serverList[i]->duration;

            QJsonObject requestInfoJson;
            requestInfoJson.insert("request",REQUEST_INFO);
            QJsonObject requestInfoDataJson;
            requestInfoDataJson.insert("TotalFee",allRoomMap[serverList[i]->roomId]->fee);
            requestInfoDataJson.insert("CurrentTemp",allRoomMap[serverList[i]->roomId]->currentTemp);
            requestInfoJson.insert("data",requestInfoDataJson);
            qDebug()<<"发送："<<serverList[i]->roomId<<requestInfoJson;
            roomIds[serverList[i]->roomId]->write(QJsonDocument(requestInfoJson).toJson(QJsonDocument::Compact)+"\n");
            roomIds[serverList[i]->roomId]->flush();
        });
    }

    // 时间片轮转
    RRTimer=new QTimer(this);
    RRTimer->setInterval(120000);
    connect(RRTimer,&QTimer::timeout,[=](){
        int replaceRoomId=serverQueue->GetServerLowestFanSpeedLongestServeTime();
        if(replaceRoomId!=-1)
        {
            QList<int> waitQueueEqualFanSpeedRoomIdList=waitQueue->equalFanSpeedRoomId(allRoomMap[replaceRoomId]->currentFanSpeed);
            if(waitQueueEqualFanSpeedRoomIdList.size()>0)
            {
                int longestWaitTime=allRoomMap[waitQueueEqualFanSpeedRoomIdList[0]]->waitServerTime;
                int longestServeTimeRoomId=waitQueueEqualFanSpeedRoomIdList[0];
                for (int i=1;i<waitQueueEqualFanSpeedRoomIdList.size();i++)
                {
                    if(longestWaitTime<allRoomMap[waitQueueEqualFanSpeedRoomIdList[i]]->waitServerTime)
                    {
                        longestWaitTime=allRoomMap[waitQueueEqualFanSpeedRoomIdList[i]]->waitServerTime;
                        longestServeTimeRoomId=waitQueueEqualFanSpeedRoomIdList[i];
                    }
                }
                serverList[allRoomMap[replaceRoomId]->serverId]->stopServe();
                sync(replaceRoomId);

                // 插入服务关闭日志
                dao->AddLog2DB(EVENT_SERVE_OFF,*allRoomMap[replaceRoomId]);

                serverQueue->AddInServerQueue(allRoomMap[longestServeTimeRoomId],allRoomMap[replaceRoomId]->serverId);
                waitQueue->MoveOutWaitQueue(longestServeTimeRoomId);
                // 发送服务开始通知包
                sendServeOnJson(longestServeTimeRoomId);

                waitQueue->AddInWaitQueue(allRoomMap[replaceRoomId]);
                serverQueue->MoveOutServerQueue(replaceRoomId);
                // 发送服务暂停通知包
                sendServePreemptJson(replaceRoomId);

                // 判断是否需要RR
                judgeRR();
            }
        }
    });
}

void Scheduler::InitSocketServer()
{
    // 开启监听所以IP，端口8888
    tcpServer->listen(QHostAddress::Any,8888);
    qDebug()<<"服务器开始监听……";
    connect(tcpServer,&QTcpServer::newConnection,this,&Scheduler::OnTcpServerConnected);
}

void Scheduler::CloseServer()// 关闭服务器
{
    if(tcpServer->isListening())
    {
        tcpServer->close();
    }
    tcpServer->deleteLater();
    qDebug()<<"关闭服务器";
}

void Scheduler::sync(int roomId)// 当房间从服务队列离开前，同步房间与服务对象信息(当前温度，费用，使用时长)，且将等待服务时长设为0
{
    allRoomMap[roomId]->originTemp=serverList[allRoomMap[roomId]->serverId]->originTemp;
    allRoomMap[roomId]->currentTemp=QString::number(serverList[allRoomMap[roomId]->serverId]->currentTemp, 'f', 2).toFloat();
    allRoomMap[roomId]->targetTemp=serverList[allRoomMap[roomId]->serverId]->targetTemp;
    allRoomMap[roomId]->currentFanSpeed=serverList[allRoomMap[roomId]->serverId]->fanSpeed;
    allRoomMap[roomId]->fee=QString::number(serverList[allRoomMap[roomId]->serverId]->fee, 'f', 2).toFloat();
    allRoomMap[roomId]->serverTime=serverList[allRoomMap[roomId]->serverId]->duration;
    allRoomMap[roomId]->waitServerTime=0;
}

void Scheduler::judgeRR()// 判断是否需要RR
{
    int replaceRoomId=serverQueue->GetServerLowestFanSpeedLongestServeTime();
    // 服务队列不为空，等待队列不为空才需要RR
    if(replaceRoomId!=-1&&serverQueue->roomMap.size()>0&&waitQueue->roomMap.size()>0)
    {
        QList<int> waitQueueEqualFanSpeedRoomIdList=waitQueue->equalFanSpeedRoomId(allRoomMap[replaceRoomId]->currentFanSpeed);
        // 等待队列有与服务队列最低优先级相同的房间，需要RR
        if(waitQueueEqualFanSpeedRoomIdList.size()>0)
        {
            if(!RRTimer->isActive())
                RRTimer->start();
            qDebug()<<"需要RR";
        }
        // 等待队列没有与服务队列最低优先级相同的房间，不需要RR
        else
        {
            if(RRTimer->isActive())
                RRTimer->stop();
            qDebug()<<"不需要RR";
        }
    }
    // 服务队列或等待队列为空，无需时间片轮转
    else
    {
        if(RRTimer->isActive())
            RRTimer->stop();
        qDebug()<<"不需要RR";
    }
}

void Scheduler::sendServePreemptJson(int roomId)// 发送抢占通知包
{
    QJsonObject requestServePreemptJson;
    requestServePreemptJson.insert("request",REQUEST_SERVE_PREEMPT);
    QJsonObject requestServePreemptDataJson;
    requestServePreemptDataJson.insert("WaitId",1);
    requestServePreemptDataJson.insert("WaitTime",120);
    requestServePreemptJson.insert("data",requestServePreemptDataJson);
    qDebug()<<"发送："<<roomId<<requestServePreemptJson;
    roomIds[roomId]->write(QJsonDocument(requestServePreemptJson).toJson(QJsonDocument::Compact)+"\n");
    roomIds[roomId]->flush();
}

void Scheduler::sendServeOnJson(int roomId)// 发送服务开启通知包
{
    QJsonObject requestServeOnJson;
    requestServeOnJson.insert("request",REQUEST_SERVE_ON);
    QJsonObject requestServeOnDataJson;
    requestServeOnDataJson.insert("ServerId",allRoomMap[roomId]->serverId);
    requestServeOnJson.insert("data",requestServeOnDataJson);
    qDebug()<<"发送："<<roomId<<requestServeOnJson;
    roomIds[roomId]->write(QJsonDocument(requestServeOnJson).toJson(QJsonDocument::Compact)+"\n");
    roomIds[roomId]->flush();
}

void Scheduler::sendServeOverJson(int roomId)// 发送服务完成通知包
{
    QJsonObject requestServeOverJson;
    requestServeOverJson.insert("request",REQUEST_SERVE_OVER);
    qDebug()<<"发送："<<roomId<<requestServeOverJson;
    roomIds[roomId]->write(QJsonDocument(requestServeOverJson).toJson(QJsonDocument::Compact)+"\n");
    roomIds[roomId]->flush();
}

void Scheduler::JsonAnalyse(QTcpSocket *tcpSocket,QJsonObject receivedJson)
{
    qDebug()<<"收到："<<receivedJson;
    int request=receivedJson["request"].toInt();
    switch (request)
    {
    // 空调开启
    case REQUEST_ON:
    {
        int roomId=receivedJson["data"].toObject()["RoomId"].toInt();
        // 回送OK应答包
        QJsonObject requestOnOkJson;
        requestOnOkJson.insert("request",REQUEST_ON);
        requestOnOkJson.insert("ACK",ACK_ON_OK);
        QJsonObject requestOnDataJson;
        requestOnDataJson.insert("DefaultTemp",defaultTemp);
        requestOnDataJson.insert("DefaultFanSpeed",defaultFanSpeed);
        requestOnDataJson.insert("Mode",mode);
        requestOnOkJson.insert("data",requestOnDataJson);
        qDebug()<<"发送："<<roomId<<requestOnOkJson;
        tcpSocket->write(QJsonDocument(requestOnOkJson).toJson(QJsonDocument::Compact)+"\n");
        tcpSocket->flush();
        // 服务已经完成后重发的请求
        if(roomIds.size()>0&&roomIds.keys().contains(roomId)&&hangUpQueue.size()>0&&hangUpQueue.keys().contains(roomId))
        {
            // 从挂机队列移出，重新调度
            RequestOn(roomId,allRoomMap[roomId]->currentFanSpeed,0);
        }
        // 新来的请求
        else
        {
            tcpSockets.insert(tcpSocket,roomId);
            roomIds.insert(roomId,tcpSocket);
            // 为每个客户端主动断开连接的信号连接
            connect(tcpSocket,&QTcpSocket::disconnected,[=](){
                // 如果是返回true就是意外断开没有移除，则移除该客户端相关信息；返回false，正常关机已经移除，再进行一遍也没问题
                if(RequestOff(roomId))
                {
                    QMessageBox::warning(this,"警告","客户端意外断开连接");
                    qDebug()<<"客户端意外断开连接";
                }
                tcpSocket->close();
                roomIds.remove(tcpSockets[tcpSocket]);
                tcpSockets.remove(tcpSocket);
                tcpSocket->deleteLater();
            });
            RequestOn(roomId,defaultFanSpeed,1);
        }
        // 判断是否需要RR
        judgeRR();
        break;
    }
    // 修改温度
    case REQUEST_CHANGE_TEMP:
    {
        int roomId=receivedJson["data"].toObject()["RoomId"].toInt();
        int targetTemp=receivedJson["data"].toObject()["TargetTemp"].toInt();
        if(targetTemp>=tempLowLimit&&targetTemp<=tempHighLimit)
        {
            // 修改温度成功，服务器回送成功包
            if(ChangeTargetTemp(roomId, targetTemp))
            {
                QJsonObject requestChangeTempOkJson;
                requestChangeTempOkJson.insert("request",REQUEST_CHANGE_TEMP);
                requestChangeTempOkJson.insert("ACK",ACK_CHANGE_TEMP_OK);
                qDebug()<<"发送："<<roomId<<requestChangeTempOkJson;
                tcpSocket->write(QJsonDocument(requestChangeTempOkJson).toJson(QJsonDocument::Compact)+"\n");
                tcpSocket->flush();
            }
            // 修改温度失败，服务器回送失败包
            else
            {
                QJsonObject requestChangeTempErrorJson;
                requestChangeTempErrorJson.insert("request",REQUEST_CHANGE_TEMP);
                requestChangeTempErrorJson.insert("ACK",ACK_CHANGE_TEMP_ERROR);
                qDebug()<<"发送："<<roomId<<requestChangeTempErrorJson;
                tcpSocket->write(QJsonDocument(requestChangeTempErrorJson).toJson(QJsonDocument::Compact)+"\n");
                tcpSocket->flush();
            }
        }
        // 目标温度超出范围，服务器回送失败包
        else
        {
            QJsonObject requestChangeTempErrorJson;
            requestChangeTempErrorJson.insert("request",REQUEST_CHANGE_TEMP);
            requestChangeTempErrorJson.insert("ACK",ACK_CHANGE_TEMP_ERROR);
            qDebug()<<"发送："<<roomId<<requestChangeTempErrorJson;
            tcpSocket->write(QJsonDocument(requestChangeTempErrorJson).toJson(QJsonDocument::Compact)+"\n");
            tcpSocket->flush();
        }
        break;
    }
    // 修改风速
    case REQUEST_CHANGE_FAN_SPEED:
    {
        int roomId=receivedJson["data"].toObject()["RoomId"].toInt();
        int fanSpeed=receivedJson["data"].toObject()["FanSpeed"].toInt();
        // 修改风速成功，服务器回送成功包
        if(ChangeFanSpeed(roomId, fanSpeed))
        {
            QJsonObject requestChangeFanSpeedOkJson;
            requestChangeFanSpeedOkJson.insert("request",REQUEST_CHANGE_FAN_SPEED);
            requestChangeFanSpeedOkJson.insert("ACK",ACK_CHANGE_FAN_SPEED_OK);
            qDebug()<<"发送："<<roomId<<requestChangeFanSpeedOkJson;
            tcpSocket->write(QJsonDocument(requestChangeFanSpeedOkJson).toJson(QJsonDocument::Compact)+"\n");
            tcpSocket->flush();
        }
        // 修改风速失败，服务器回送失败包
        else
        {
            QJsonObject requestChangeFanSpeedErrorJson;
            requestChangeFanSpeedErrorJson.insert("request",REQUEST_CHANGE_FAN_SPEED);
            requestChangeFanSpeedErrorJson.insert("ACK",ACK_CHANGE_FAN_SPEED_ERROR);
            qDebug()<<"发送："<<roomId<<requestChangeFanSpeedErrorJson;
            tcpSocket->write(QJsonDocument(requestChangeFanSpeedErrorJson).toJson(QJsonDocument::Compact)+"\n");
            tcpSocket->flush();
        }
        // 判断是否需要RR
        judgeRR();
        break;
    }
    // 空调关闭
    case REQUEST_OFF:
    {
        int roomId=receivedJson["data"].toObject()["RoomId"].toInt();
        // 服务关闭成功，回送关闭服务成功ACK包
        if(RequestOff(roomId))
        {
            QJsonObject requestOffOkJson;
            requestOffOkJson.insert("request",REQUEST_OFF);
            requestOffOkJson.insert("ACK",ACK_OFF_OK);
            qDebug()<<"发送："<<roomId<<requestOffOkJson;
            tcpSocket->write(QJsonDocument(requestOffOkJson).toJson(QJsonDocument::Compact)+"\n");
            tcpSocket->flush();
            // 客户端收到这个包后会主动断开连接，此时服务器触发QTcpSocket::disconnected信号
        }
        // 服务关闭失败，回送关闭服务失败ACK包
        else
        {
            QJsonObject requestOffErrorJson;
            requestOffErrorJson.insert("request",REQUEST_OFF);
            requestOffErrorJson.insert("ACK",ACK_OFF_ERROR);
            qDebug()<<"发送："<<roomId<<requestOffErrorJson;
            tcpSocket->write(QJsonDocument(requestOffErrorJson).toJson(QJsonDocument::Compact)+"\n");
            tcpSocket->flush();
        }
        // 判断是否需要RR
        judgeRR();
        break;
    }
    // 客户端发送的服务启动ACK包
    case REQUEST_SERVE_ON:
    {
        int ACK=receivedJson["ACK"].toInt();
        // 服务启动-OK
        // ACK_SERVE_ON_OK
        if(ACK==ACK_SERVE_ON_OK)
        {
            // 取出客户端发送的当前温度，赋值给房间和服务对象，此时服务对象正式开启服务
            allRoomMap[tcpSockets[tcpSocket]]->currentTemp=receivedJson["data"].toObject()["CurrentTemp"].toDouble();
            // 挂机的重发请求，初始温度不变；新请求，初始温度设为当前温度
            if(allRoomMap[tcpSockets[tcpSocket]]->state==ROOM_STATE_ON)
            {
                allRoomMap[tcpSockets[tcpSocket]]->originTemp=receivedJson["data"].toObject()["CurrentTemp"].toDouble();
            }

            allRoomMap[tcpSockets[tcpSocket]]->state=ROOM_STATE_SERVE;
            int startServerId=allRoomMap[tcpSockets[tcpSocket]]->serverId;
            if(startServerId!=-1)
            {
                serverList[startServerId]->SetServer(
                            tcpSockets[tcpSocket],
                            allRoomMap[tcpSockets[tcpSocket]]->originTemp,
                            allRoomMap[tcpSockets[tcpSocket]]->targetTemp,
                            allRoomMap[tcpSockets[tcpSocket]]->currentTemp,
                            allRoomMap[tcpSockets[tcpSocket]]->currentFanSpeed,
                            allRoomMap[tcpSockets[tcpSocket]]->fee,
                            allRoomMap[tcpSockets[tcpSocket]]->serverTime);

                serverList[allRoomMap[tcpSockets[tcpSocket]]->serverId]->startServe();

                // 插入服务开启日志
                dao->AddLog2DB(EVENT_SERVE_ON,*allRoomMap[tcpSockets[tcpSocket]]);
            }
            else
            {
                QMessageBox::warning(this,"警告","获取服务器ID错误");
                qDebug()<<"startServerId:获取服务器ID错误";
            }

        }
        // 服务启动-ERROR
        // ACK_SERVE_ON_ERROR
        else
        {
            QMessageBox::warning(this,"房间服务启动失败","ACK_SERVE_ON_ERROR");
            qDebug()<<"房间服务启动-ERROR";
        }
        break;
    }
    // 客户端发送的服务完成ACK包
    case REQUEST_SERVE_OVER:
    {
        int ACK=receivedJson["ACK"].toInt();
        // 服务完成-OK
        // ACK_SERVE_OVER_OK
        if(ACK==ACK_SERVE_OVER_OK)
        {
            qDebug()<<"客户端成功启动回温程序";
        }
        // 服务完成-ERROR
        // ACK_SERVE_OVER_ERROR
        else
        {
            QMessageBox::warning(this,"客户端成功启动回温程序失败","ACK_SERVE_OVER_ERROR");
            qDebug()<<"房间服务完成-ERROR";
        }
        break;
    }
    // 服务暂停（被抢占或者请求开启服务没有满足）
    case REQUEST_SERVE_PREEMPT:
    {
        int ACK=receivedJson["ACK"].toInt();
        // 服务暂停（被抢占）-OK
        // ACK_SERVE_PREEMPT_OK
        if(ACK==ACK_SERVE_PREEMPT_OK)
        {
            // 取出客户端发送的当前温度，赋值给房间，此时房间正式进入等待状态
            allRoomMap[tcpSockets[tcpSocket]]->currentTemp=receivedJson["data"].toObject()["CurrentTemp"].toDouble();
            // 挂机的重发请求，初始温度不变；新请求，初始温度设为当前温度
            if(allRoomMap[tcpSockets[tcpSocket]]->state==ROOM_STATE_ON)
            {
                allRoomMap[tcpSockets[tcpSocket]]->originTemp=receivedJson["data"].toObject()["CurrentTemp"].toDouble();
            }
            allRoomMap[tcpSockets[tcpSocket]]->state=ROOM_STATE_WAIT;
        }
        // 服务暂停（被抢占）-ERROR
        // ACK_SERVE_PREEMPT_ERROR
        else
        {
            QMessageBox::warning(this,"房间被抢占错误","ACK_SERVE_PREEMPT_ERROR");
            qDebug()<<"房间被抢占，进入等待队列，服务暂停（被抢占）-ERROR";
        }
        break;
    }
    // 客户端的回温包
    case REQUEST_REWORM_TEMP:
    {
        allRoomMap[tcpSockets[tcpSocket]]->currentTemp=receivedJson["data"].toObject()["CurrentTemp"].toDouble();
        break;
    }
    // 其他错误请求码
    default:
        QMessageBox::warning(this,"Json包请求码错误","request="+QString(request));
        qDebug()<<"Json包请求码错误：request="<<request;
        break;
    }
}

void Scheduler::OnTcpServerConnected()
{
    qDebug()<<"建立连接";
    QTcpSocket *tcpSocket=tcpServer->nextPendingConnection();// 取出通信套接字
    qDebug()<<"ip="<<tcpSocket->peerAddress().toString()<<",port="<<tcpSocket->peerPort();
    connect(tcpSocket,&QTcpSocket::readyRead,[=](){
        QByteArray receivedPackage=tcpSocket->readLine();
        while (!receivedPackage.isEmpty())
        {
            QJsonParseError parseJsonErr;
            QJsonDocument document = QJsonDocument::fromJson(receivedPackage,&parseJsonErr);
            if(!(parseJsonErr.error == QJsonParseError::NoError))
            {
                QMessageBox::warning(this,"解析json文件错误！","解析json文件错误！");
                qDebug()<<"解析json文件错误！";
                return;
            }
            JsonAnalyse(tcpSocket,document.object());
            receivedPackage=tcpSocket->readLine();
        }
    });
}

//返回>=0的数字，代表被替换的房间ID；返回==-1，该房间等待；返回-2，出错
int Scheduler::GetReplaceRoomId(int fanSpeed)
{
    QList<int> lowerFanSpeedRoomIdList=serverQueue->GetServerLowerFanSpeed(fanSpeed);
    // 2.1 如果请求服务的风速大于部分服务对象的风速的大小，启动优先级调度
    if(lowerFanSpeedRoomIdList.size()!=0)
    {
        // 2.1.1 如果只有一个服务对象的风速低于请求对象，则选择该房间
        if(lowerFanSpeedRoomIdList.size()==1)
        {
            return lowerFanSpeedRoomIdList[0];
        }
        // 2.1.2/3 如果有多个服务对象的风速低于请求对象
        //  lowerFanSpeedRoomIdList.size()>1
        else
        {
            // 获取最小风速房间ID列表
            int lowestFanSpeed=allRoomMap[lowerFanSpeedRoomIdList[0]]->currentFanSpeed;
            QList<int> lowestFanSpeedRoomIdList;
            lowestFanSpeedRoomIdList.append(lowerFanSpeedRoomIdList[0]);
            for (int i=1;i<lowerFanSpeedRoomIdList.size();i++)
            {
                if(lowestFanSpeed>allRoomMap[lowerFanSpeedRoomIdList[i]]->currentFanSpeed)
                {
                    lowestFanSpeed=allRoomMap[lowerFanSpeedRoomIdList[i]]->currentFanSpeed;
                    lowestFanSpeedRoomIdList.clear();
                    lowestFanSpeedRoomIdList.append(lowerFanSpeedRoomIdList[i]);
                }
                else if (lowestFanSpeed==allRoomMap[lowerFanSpeedRoomIdList[i]]->currentFanSpeed)
                {
                    lowestFanSpeedRoomIdList.append(lowerFanSpeedRoomIdList[i]);
                }
            }
            // 2.1.2 如果有多个服务对象的风速相等且低于请求对象，则选择服务时长最大的房间
            if(lowestFanSpeedRoomIdList.size()>1)
            {
                float longestServeTime=allRoomMap[lowestFanSpeedRoomIdList[0]]->serverTime;
                int longestServeTimeRoomId=lowestFanSpeedRoomIdList[0];
                for (int i=1;i<lowestFanSpeedRoomIdList.size();i++)
                {
                    if(longestServeTime<allRoomMap[lowestFanSpeedRoomIdList[i]]->serverTime)
                    {
                        longestServeTime=allRoomMap[lowestFanSpeedRoomIdList[i]]->serverTime;
                        longestServeTimeRoomId=lowestFanSpeedRoomIdList[i];
                    }
                }
                return longestServeTimeRoomId;
            }
            // 2.1.3 如果多个服务对象的风速低于请求风速，且风速不相等，则选择风速最低的房间
            // lowestFanSpeedRoomIdList.size()==1，不可能=0
            else
            {
                return lowestFanSpeedRoomIdList[0];
            }
        }
    }
    // 2.2/3 如果请求服务的风速<=所有服务对象的风速的大小，直接加入等待队列
    // lowerFanSpeedRoomIdList.size()==0
    else
    {
        return -1;
    }
}

int Scheduler::RequestOn(int roomId,int fanSpeed,int flag)
{
    if(serverQueue->roomMap.size()<3)
    {
        // 找到第一个空闲的server，也就是状态为将房间与服务对象绑定
        for (int i=0;i<serverList.size();i++)
        {
            if(serverList[i]->state==SERVER_STATE_FREE)
            {
                // 新的开机请求
                if(flag==1)
                {
                    serverQueue->AddInServerQueue(roomId,i);
                    allRoomMap.insert(roomId,serverQueue->roomMap[roomId]);

                    // 插入开机日志
                    dao->AddLog2DB(EVENT_REQUEST_ON,*allRoomMap[roomId]);
                }
                // 重发的请求
                else
                {
                    serverQueue->AddInServerQueue(allRoomMap[roomId],i);
                    hangUpQueue.remove(roomId);
                }
                // 发送服务开始通知包
                sendServeOnJson(roomId);
                serverList[i]->InitRoomServer(roomId);
                return 1;
            }
        }
        QMessageBox::warning(this,"调度错误！","没有空闲的服务对象");
        qDebug()<<"RequestOn:没有空闲的服务对象";
        return -1;
    }
    else if(serverQueue->roomMap.size()==3)
    {
        int replaceRoomId=GetReplaceRoomId(fanSpeed);
        // 返回>=0的数字，代表被替换的房间ID
        if(replaceRoomId>=0)
        {
            serverList[allRoomMap[replaceRoomId]->serverId]->stopServe();
            sync(replaceRoomId);

            // 插入服务关闭日志
            dao->AddLog2DB(EVENT_SERVE_OFF,*allRoomMap[replaceRoomId]);

            // 新的开机请求
            if(flag==1)
            {
                serverQueue->AddInServerQueue(roomId,allRoomMap[replaceRoomId]->serverId);
                allRoomMap.insert(roomId,serverQueue->roomMap[roomId]);
                // 发送服务开始通知包
                sendServeOnJson(roomId);

                waitQueue->AddInWaitQueue(allRoomMap[replaceRoomId]);
                serverQueue->MoveOutServerQueue(replaceRoomId);
                // 发送服务暂停通知包
                sendServePreemptJson(replaceRoomId);

                // 插入开机日志
                dao->AddLog2DB(EVENT_REQUEST_ON,*allRoomMap[roomId]);
            }
            // 重发的请求
            else
            {
                serverQueue->AddInServerQueue(allRoomMap[roomId],allRoomMap[replaceRoomId]->serverId);
                hangUpQueue.remove(roomId);
                // 发送服务开始通知包
                sendServeOnJson(roomId);

                waitQueue->AddInWaitQueue(allRoomMap[replaceRoomId]);
                serverQueue->MoveOutServerQueue(replaceRoomId);
                // 发送服务暂停通知包
                sendServePreemptJson(replaceRoomId);
            }
            return 1;
        }
        // 返回0，该房间等待
        else
        {
            // 新的开机请求
            if(flag==1)
            {
                waitQueue->AddInWaitQueue(roomId);
                allRoomMap.insert(roomId,waitQueue->roomMap[roomId]);

                // 插入开机日志
                dao->AddLog2DB(EVENT_REQUEST_ON,*allRoomMap[roomId]);
            }
            // 重发的请求
            else
            {
                waitQueue->AddInWaitQueue(allRoomMap[roomId]);
                hangUpQueue.remove(roomId);
            }
            // 发送服务暂停通知包
            sendServePreemptJson(roomId);
            return 0;
        }
    }
    // 错误，其他对象正在处理服务队列
    else
    {
        QMessageBox::warning(this,"警告","其他对象正在处理服务队列");
        qDebug()<<"RequestOn:其他对象正在处理服务队列";
        return -1;
    }
}

bool Scheduler::ChangeTargetTemp(int roomId,int targetTemp)
{
    for (QMap<int,Room*>::iterator it=serverQueue->roomMap.begin();it!=serverQueue->roomMap.end();it++)
    {
        if(it.key()==roomId)
        {
            it.value()->ChangeTargetTemp(targetTemp);
            if(serverList[it.value()->serverId]->SChangeTargetTemp(roomId,targetTemp))
            {
                // 插入修改温度日志
                dao->AddLog2DB(EVENT_CHANGE_TEMP,*it.value());

                // 判断是否到达指定温度，关闭或开启服务
                // 在服务队列
                if((mode==SERVER_MODE_COLD&&allRoomMap[roomId]->targetTemp>=allRoomMap[roomId]->currentTemp)||
                   (mode==SERVER_MODE_WARM&&allRoomMap[roomId]->targetTemp<=allRoomMap[roomId]->currentTemp))
                {
                    serverList[allRoomMap[roomId]->serverId]->stopServe();
                    sync(roomId);
                    RequestOver(roomId);
                }

                return true;
            }
            else
            {
                QMessageBox::warning(this,"修改温度错误","修改温度错误");
                qDebug()<<"ChangeTargetTemp:修改温度错误";
                return false;
            }
        }
    }
    for (QMap<int,Room*>::iterator it=waitQueue->roomMap.begin();it!=waitQueue->roomMap.end();it++)
    {
        if(it.key()==roomId)
        {
            it.value()->ChangeTargetTemp(targetTemp);

            // 插入修改温度日志
            dao->AddLog2DB(EVENT_CHANGE_TEMP,*it.value());

            // 判断是否到达指定温度，关闭或开启服务
            // 在等待队列
            if((mode==SERVER_MODE_COLD&&allRoomMap[roomId]->targetTemp>=allRoomMap[roomId]->currentTemp)||
               (mode==SERVER_MODE_WARM&&allRoomMap[roomId]->targetTemp<=allRoomMap[roomId]->currentTemp))
            {
                allRoomMap[roomId]->state=ROOM_STATE_HANG_UP;
                allRoomMap[roomId]->serverTime=0;
                allRoomMap[roomId]->waitServerTime=0;
                allRoomMap[roomId]->serverId=-1;
                allRoomMap[roomId]->waitId=-1;
                // 加入挂机队列
                hangUpQueue.insert(roomId,allRoomMap[roomId]);
                // 发送服务完成通知包
                sendServeOverJson(roomId);
                waitQueue->MoveOutWaitQueue(roomId);
                // 判断是否需要RR
                judgeRR();
            }

            return true;
        }
    }
    for (QMap<int,Room*>::iterator it=hangUpQueue.begin();it!=hangUpQueue.end();it++)
    {
        if(it.key()==roomId)
        {
            it.value()->ChangeTargetTemp(targetTemp);

            // 插入修改温度日志
            dao->AddLog2DB(EVENT_CHANGE_TEMP,*it.value());

            return true;
        }
    }
    return false;
}

bool Scheduler::ChangeFanSpeed(int roomId,int fanSpeed)
{
    for (QMap<int,Room*>::iterator it=serverQueue->roomMap.begin();it!=serverQueue->roomMap.end();it++)
    {
        if(it.key()==roomId)
        {
            it.value()->ChangeFanSpeed(fanSpeed);
            if(serverList[it.value()->serverId]->SChangeFanSpeed(roomId,fanSpeed))
            {
                // 插入修改风速日志
                dao->AddLog2DB(EVENT_CHANGE_FAN_SPEED,*it.value());

                // 重新调度
                // 找等待队列有没有比这个风速高的，有就选择等待时间最长的替换
                QList<int> higherFanSpeedRoomIdList=waitQueue->higherFanSpeed(fanSpeed);
                if(higherFanSpeedRoomIdList.size()>=1)
                {
                    int longestWaitServeTime=allRoomMap[higherFanSpeedRoomIdList[0]]->waitServerTime;
                    int longestWaitServeTimeRoomId=higherFanSpeedRoomIdList[0];
                    for (int i=1;i<higherFanSpeedRoomIdList.size();i++)
                    {
                        if(longestWaitServeTime<allRoomMap[higherFanSpeedRoomIdList[i]]->waitServerTime)
                        {
                            longestWaitServeTime=allRoomMap[higherFanSpeedRoomIdList[i]]->waitServerTime;
                            longestWaitServeTimeRoomId=higherFanSpeedRoomIdList[i];
                        }
                    }
                    serverList[allRoomMap[roomId]->serverId]->stopServe();
                    sync(roomId);

                    // 插入服务关闭日志
                    dao->AddLog2DB(EVENT_SERVE_OFF,*allRoomMap[roomId]);

                    serverQueue->AddInServerQueue(allRoomMap[longestWaitServeTimeRoomId],allRoomMap[roomId]->serverId);
                    waitQueue->MoveOutWaitQueue(longestWaitServeTimeRoomId);
                    // 发送服务开始通知包
                    sendServeOnJson(longestWaitServeTimeRoomId);

                    waitQueue->AddInWaitQueue(allRoomMap[roomId]);
                    serverQueue->MoveOutServerQueue(roomId);
                    // 发送服务暂停通知包
                    sendServePreemptJson(roomId);

                }
                return true;
            }
            else
            {
                QMessageBox::warning(this,"修改风速错误","修改风速错误");
                qDebug()<<"ChangeFanSpeed:修改风速错误";
                return false;
            }
        }        
    }
    for (QMap<int,Room*>::iterator it=waitQueue->roomMap.begin();it!=waitQueue->roomMap.end();it++)
    {
        if(it.key()==roomId)
        {
            it.value()->ChangeFanSpeed(fanSpeed);

            // 插入修改风速日志
            dao->AddLog2DB(EVENT_CHANGE_FAN_SPEED,*it.value());

            // 重新调度
            // 找服务队列有没有比这个风速低的，有就选择服务时间最长的替换
            QList<int> lowerFanSpeedRoomIdList=serverQueue->GetServerLowerFanSpeed(fanSpeed);
            if(lowerFanSpeedRoomIdList.size()>=1)
            {
                float longestServeTime=allRoomMap[lowerFanSpeedRoomIdList[0]]->serverTime;
                int longestServeTimeRoomId=lowerFanSpeedRoomIdList[0];
                for (int i=1;i<lowerFanSpeedRoomIdList.size();i++)
                {
                    if(longestServeTime<allRoomMap[lowerFanSpeedRoomIdList[i]]->serverTime)
                    {
                        longestServeTime=allRoomMap[lowerFanSpeedRoomIdList[i]]->serverTime;
                        longestServeTimeRoomId=lowerFanSpeedRoomIdList[i];
                    }
                }
                serverList[allRoomMap[longestServeTimeRoomId]->serverId]->stopServe();
                sync(longestServeTimeRoomId);

                // 插入服务关闭日志
                dao->AddLog2DB(EVENT_SERVE_OFF,*allRoomMap[roomId]);

                serverQueue->AddInServerQueue(allRoomMap[roomId],allRoomMap[longestServeTimeRoomId]->serverId);
                waitQueue->MoveOutWaitQueue(roomId);
                // 发送服务开始通知包
                sendServeOnJson(roomId);

                waitQueue->AddInWaitQueue(allRoomMap[longestServeTimeRoomId]);
                serverQueue->MoveOutServerQueue(longestServeTimeRoomId);
                // 发送服务暂停通知包
                sendServePreemptJson(longestServeTimeRoomId);

            }
            return true;
        }
    }
    for (QMap<int,Room*>::iterator it=hangUpQueue.begin();it!=hangUpQueue.end();it++)
    {
        if(it.value()->roomId==roomId)
        {
            it.value()->ChangeFanSpeed(fanSpeed);

            // 插入修改风速日志
            dao->AddLog2DB(EVENT_CHANGE_FAN_SPEED,*it.value());

            // 在挂机队列，修改风速不会影响调度

            return true;
        }
    }

    return false;
}

bool Scheduler::RequestOff(int roomId)
{
    // 分别从不同的列表中移除
    for (QMap<int,Room*>::iterator it=serverQueue->roomMap.begin();it!=serverQueue->roomMap.end();it++)
    {
        if(it.key()==roomId)
        {
            serverList[it.value()->serverId]->stopServe();
            sync(roomId);

            // 插入服务关闭日志
            dao->AddLog2DB(EVENT_SERVE_OFF,*it.value());
            // 插入关机日志
            dao->AddLog2DB(EVENT_REQUEST_OFF,*it.value());

            int startRoomId=waitQueue->GetMaxWaitTime();
            if(startRoomId!=-1)
            {
                serverQueue->AddInServerQueue(allRoomMap[startRoomId],it.value()->serverId);
                waitQueue->MoveOutWaitQueue(startRoomId);
                sendServeOnJson(startRoomId);
            }
            serverQueue->MoveOutServerQueue(roomId);

            // 关机，从总列表中移除
            allRoomMap.remove(roomId);

            return true;
        }
    }
    for (QMap<int,Room*>::iterator it=waitQueue->roomMap.begin();it!=waitQueue->roomMap.end();it++)
    {
        if(it.key()==roomId)
        {
            // 插入关机日志
            dao->AddLog2DB(EVENT_REQUEST_OFF,*it.value());

            waitQueue->MoveOutWaitQueue(roomId);

            // 关机，从总列表中移除
            allRoomMap.remove(roomId);

            return true;
        }
    }
    for (QMap<int,Room*>::iterator it=hangUpQueue.begin();it!=hangUpQueue.end();it++)
    {
        if(it.value()->roomId==roomId)
        {
            // 插入关机日志
            dao->AddLog2DB(EVENT_REQUEST_OFF,*it.value());

            hangUpQueue.remove(it.key());

            // 关机，从总列表中移除
            allRoomMap.remove(roomId);

            return true;
        }
    }
    return false;
}

void Scheduler::RequestOver(int roomId)
{
    // 重新调度
    int startRoomId=waitQueue->GetMaxWaitTime();
    if(startRoomId!=-1)
    {
        serverQueue->AddInServerQueue(allRoomMap[startRoomId],allRoomMap[roomId]->serverId);
        waitQueue->MoveOutWaitQueue(startRoomId);
        sendServeOnJson(startRoomId);
    }
    allRoomMap[roomId]->state=ROOM_STATE_HANG_UP;
    allRoomMap[roomId]->serverTime=0;
    allRoomMap[roomId]->waitServerTime=0;
    allRoomMap[roomId]->serverId=-1;
    allRoomMap[roomId]->waitId=-1;
    // 插入服务关闭日志
    dao->AddLog2DB(EVENT_SERVE_OFF,*allRoomMap[roomId]);
    // 加入挂机队列
    hangUpQueue.insert(roomId,allRoomMap[roomId]);
    serverQueue->MoveOutServerQueue(roomId);
    // 发送服务完成通知包
    sendServeOverJson(roomId);
    // 判断是否需要RR
    judgeRR();
}
