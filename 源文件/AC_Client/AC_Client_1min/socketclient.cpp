#include "socketclient.h"

SocketClient::SocketClient(QWidget *parent)
    : QWidget(parent)
{
    tcpSocket=new QTcpSocket(this);
    connect(tcpSocket,&QTcpSocket::connected,this,&SocketClient::OnTcpClientConnected);
    connect(tcpSocket,&QTcpSocket::disconnected,this,&SocketClient::OnTcpClientDisconnected);
}

SocketClient::~SocketClient()
{

}

void SocketClient::JsonAnalyse(QJsonObject receivedJson)
{
    qDebug()<<"收到："<<receivedJson;
    int request=receivedJson["request"].toInt();
    switch (request)
    {
    // 空调开启
    case REQUEST_ON:
    {
        int ACK=receivedJson["ACK"].toInt();
        // ACK_ON_OK，收到了默认参数包
        if(ACK==ACK_ON_OK)
        {
            // 判断是否是重发请求的回复
            // 关机状态，为新请求
            if(state==STATE_OFF)
            {
                defaultTemp=receivedJson["data"].toObject()["DefaultTemp"].toInt();
                defaultFanSpeed=receivedJson["data"].toObject()["DefaultFanSpeed"].toInt();
                mode=receivedJson["data"].toObject()["Mode"].toInt();
                emit signalShowDefaultPara();
            }
            // 挂机状态和关机状态都要修改状态为开启
            state=STATE_ON;
            emit signalChangeState();
        }
        // ACK_ON_ERROR，错误
        else
        {
            QMessageBox::warning(this,"警告","调度正忙或出错，请稍后再试");
            qDebug()<<"开机调度-ERROR";
        }
        break;
    }
    // 修改温度
    case REQUEST_CHANGE_TEMP:
    {
        int ACK=receivedJson["ACK"].toInt();
        // ACK_CHANGE_TEMP_OK，收到了修改温度成功应答包
        if(ACK==ACK_CHANGE_TEMP_OK)
        {
            QMessageBox::information(this,"提示","修改温度成功");
            qDebug()<<"修改温度成功";
            emit signalTempChangeOk();
        }
        // ACK_CHANGE_TEMP_ERROR，错误
        else
        {
            QMessageBox::warning(this,"修改温度失败","温度范围超出限制");
            qDebug()<<"修改温度应答包-ERROR";
        }
        break;
    }
    // 修改风速
    case REQUEST_CHANGE_FAN_SPEED:
    {
        int ACK=receivedJson["ACK"].toInt();
        // ACK_CHANGE_FAN_SPEED_OK，收到了修改风速成功应答包
        if(ACK==ACK_CHANGE_FAN_SPEED_OK)
        {
            QMessageBox::information(this,"提示","修改风速成功");
            qDebug()<<"修改风速成功";
            emit signalFanSpeedChangeOk();
        }
        // ACK_CHANGE_FAN_SPEED_ERROR，错误
        else
        {
            QMessageBox::warning(this,"修改风速失败","ACK_CHANGE_FAN_SPEED_ERROR");
            qDebug()<<"修改风速应答包-ERROR";
        }
        break;
    }
    // 空调关闭
    case REQUEST_OFF:
    {
        int ACK=receivedJson["ACK"].toInt();
        // ACK_OFF_OK，收到了关机成功应答包
        if(ACK==ACK_OFF_OK)
        {
            // 断开连接，删除socket
            qDebug()<<"客户端断开与服务器的连接……";
            tcpSocket->disconnectFromHost();
            state=STATE_OFF;
            emit signalChangeState();
            QMessageBox::information(this,"提示","关机成功");
            qDebug()<<"关机成功";
        }
        // ACK_OFF_ERROR，错误
        else
        {
            QMessageBox::warning(this,"关机失败","ACK_OFF_ERROR");
            qDebug()<<"关机应答包-ERROR";
        }
        break;
    }
    // 服务器发送的服务启动包
    case REQUEST_SERVE_ON:
    {
        serverId=receivedJson["data"].toObject()["ServerId"].toInt();
        // 服务启动-OK
        state=STATE_SERVE;
        emit signalChangeState();
        emit signalTempFanButtonEnable();
        // 回送ACK_SERVE_ON_OK包，返回客户端当前温度
        QJsonObject requestServeOnOkJson;
        requestServeOnOkJson.insert("request",REQUEST_SERVE_ON);
        requestServeOnOkJson.insert("ACK",ACK_SERVE_ON_OK);
        QJsonObject requestServeOnOkDataJson;
        requestServeOnOkDataJson.insert("CurrentTemp",currentTemp);
        requestServeOnOkJson.insert("data",requestServeOnOkDataJson);
        qDebug()<<"发送："<<requestServeOnOkJson;
        tcpSocket->write(QJsonDocument(requestServeOnOkJson).toJson(QJsonDocument::Compact)+"\n");
        tcpSocket->flush();
        break;
    }
    // 服务器发送的服务完成包
    case REQUEST_SERVE_OVER:
    {
        // 修改当前状态为挂机状态
        state=STATE_HANG_UP;
        emit signalChangeState();
        // 服务完成，启动回温程序
        emit signalStartRewormProgrom();
        // 启动回温程序成功
        qDebug()<<"启动回温程序成功";
        // 回送ACK_SERVE_OVER_OK包
        QJsonObject requestServeOverOkJson;
        requestServeOverOkJson.insert("request",REQUEST_SERVE_OVER);
        requestServeOverOkJson.insert("ACK",ACK_SERVE_OVER_OK);
        qDebug()<<"发送："<<requestServeOverOkJson;
        tcpSocket->write(QJsonDocument(requestServeOverOkJson).toJson(QJsonDocument::Compact)+"\n");
        tcpSocket->flush();

        // 假定不可能发生下面的情况
        // 启动回温程序失败
        // 回送ACK_SERVE_OVER_ERROR包
//        QMessageBox::warning(this,"启动回温程序失败","ACK_SERVE_OVER_ERROR");
//        qDebug()<<"房间启动回温程序-ERROR";
//        // 回送ACK_SERVE_OVER_ERROR包
//        QJsonObject requestServeOverErrorJson;
//        requestServeOverErrorJson.insert("request",REQUEST_SERVE_OVER);
//        requestServeOverErrorJson.insert("ACK",ACK_SERVE_OVER_ERROR);
//        qDebug()<<"发送："<<requestServeOverErrorJson;
//        tcpSocket->write(QJsonDocument(requestServeOverErrorJson).toJson(QJsonDocument::Compact)+"\n");
//        tcpSocket->flush();

        break;
    }
    // 服务器发送的服务暂停（被抢占或者请求开启服务没有满足）包
    case REQUEST_SERVE_PREEMPT:
    {
        // 服务暂停，温度不变，等待，修改状态为等待
        state=STATE_WAIT;
        emit signalChangeState();
        qDebug()<<"服务暂停成功";
        // 回送ACK_SERVE_PREEMPT_OK包，返回客户端当前温度
        QJsonObject requestServePreemptOkJson;
        requestServePreemptOkJson.insert("request",REQUEST_SERVE_PREEMPT);
        requestServePreemptOkJson.insert("ACK",ACK_SERVE_PREEMPT_OK);
        QJsonObject requestServePreemptOkDataJson;
        requestServePreemptOkDataJson.insert("CurrentTemp",currentTemp);
        requestServePreemptOkJson.insert("data",requestServePreemptOkDataJson);
        qDebug()<<"发送："<<requestServePreemptOkJson;
        tcpSocket->write(QJsonDocument(requestServePreemptOkJson).toJson(QJsonDocument::Compact)+"\n");
        tcpSocket->flush();
        break;
    }
    // 当前温度和总费用心跳包
    case REQUEST_INFO:
    {
        // 更新当前温度和总费用并刷新显示
        totalFee=receivedJson["data"].toObject()["TotalFee"].toDouble();
        currentTemp=receivedJson["data"].toObject()["CurrentTemp"].toDouble();
        emit signalRefreshTempFee();
        break;
    }
    // 其他错误请求码
    default:
        QMessageBox::warning(this,"Json包请求码错误","request="+QString(request));
        qDebug()<<"Json包请求码错误：request="<<request;
        break;
    }
}

void SocketClient::OnTcpClientConnected()
{
    qDebug()<<"连接成功";
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
            JsonAnalyse(document.object());
            receivedPackage=tcpSocket->readLine();
        }
    });

}

void SocketClient::OnTcpClientDisconnected()// 断开连接
{
    QMessageBox::information(this,"通知","与服务器的连接断开");
    qDebug()<<"断开连接";
    currentTemp=originTemp;
    totalFee=0;
    state=STATE_OFF;

    emit signalChangeState();
    emit signalTcpDisconnected();
}

void SocketClient::SendRequestOn(int roomId)// 发送开机请求包
{
    QJsonObject requestOnJson;
    requestOnJson.insert("request",REQUEST_ON);
    QJsonObject requestOnDataJson;
    requestOnDataJson.insert("RoomId",roomId);
    requestOnJson.insert("data",requestOnDataJson);
    qDebug()<<"发送："<<requestOnJson;
    tcpSocket->write(QJsonDocument(requestOnJson).toJson(QJsonDocument::Compact)+"\n");
    tcpSocket->flush();
}

void SocketClient::SendRequestChangeTargetTemp(int roomId,int targetTemp)// 发送修改温度请求包
{
    QJsonObject requestChangeTargetTempJson;
    requestChangeTargetTempJson.insert("request",REQUEST_CHANGE_TEMP);
    QJsonObject requestChangeTargetTempDataJson;
    requestChangeTargetTempDataJson.insert("RoomId",roomId);
    requestChangeTargetTempDataJson.insert("TargetTemp",targetTemp);
    requestChangeTargetTempJson.insert("data",requestChangeTargetTempDataJson);
    qDebug()<<"发送："<<requestChangeTargetTempJson;
    tcpSocket->write(QJsonDocument(requestChangeTargetTempJson).toJson(QJsonDocument::Compact)+"\n");
    tcpSocket->flush();
}

void SocketClient::SendRequestChangeFanSpeed(int roomId,int fanSpeed)// 发送修改风速请求包
{
    QJsonObject requestChangeFanSpeedJson;
    requestChangeFanSpeedJson.insert("request",REQUEST_CHANGE_FAN_SPEED);
    QJsonObject requestChangeFanSpeedDataJson;
    requestChangeFanSpeedDataJson.insert("RoomId",roomId);
    requestChangeFanSpeedDataJson.insert("FanSpeed",fanSpeed);
    requestChangeFanSpeedJson.insert("data",requestChangeFanSpeedDataJson);
    qDebug()<<"发送："<<requestChangeFanSpeedJson;
    tcpSocket->write(QJsonDocument(requestChangeFanSpeedJson).toJson(QJsonDocument::Compact)+"\n");
    tcpSocket->flush();
}

void SocketClient::SendRequestOff(int roomId)// 发送关机请求包
{
    QJsonObject requestOffJson;
    requestOffJson.insert("request",REQUEST_OFF);
    QJsonObject requestOffDataJson;
    requestOffDataJson.insert("RoomId",roomId);
    requestOffJson.insert("data",requestOffDataJson);
    qDebug()<<"发送："<<requestOffJson;
    tcpSocket->write(QJsonDocument(requestOffJson).toJson(QJsonDocument::Compact)+"\n");
    tcpSocket->flush();
}

void SocketClient::SendRewormTemp()// 回温温度心跳包
{
    QJsonObject requestRewormTempJson;
    requestRewormTempJson.insert("request",REQUEST_REWORM_TEMP);
    QJsonObject requestRewormTempDataJson;
    requestRewormTempDataJson.insert("CurrentTemp",currentTemp);
    requestRewormTempJson.insert("data",requestRewormTempDataJson);
    qDebug()<<"发送："<<requestRewormTempJson;
    tcpSocket->write(QJsonDocument(requestRewormTempJson).toJson(QJsonDocument::Compact)+"\n");
    tcpSocket->flush();
}
