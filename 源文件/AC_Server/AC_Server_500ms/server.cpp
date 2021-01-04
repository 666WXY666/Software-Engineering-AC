#include "server.h"

Server::Server(int serverId, int mode,int defaultTargetTemp,int defaultFanSpeed,float feeRate,float fan[],QWidget *parent) : QWidget(parent)
{
    this->serverId = serverId;

    SetACPara(mode,defaultTargetTemp,defaultFanSpeed,feeRate,fan);

    durationTimer=new QTimer(this);
    durationTimer->setInterval(500);
    connect(durationTimer,&QTimer::timeout,[=](){
        duration+=0.5;
        if(mode==SERVER_MODE_WARM)
        {
            if(currentTemp<targetTemp)
            {
                if(fanSpeed==0)
                {
                    this->currentTemp+=fan[0]/120;
                    GetFee(fan[0]);
                }
                else if(fanSpeed==1)
                {
                    this->currentTemp+=fan[1]/120;
                    GetFee(fan[1]);
                }
                else if(fanSpeed==2)
                {
                    this->currentTemp+=fan[2]/120;
                    GetFee(fan[2]);
                }
                emit signalSendInfo();
            }
            // 判断加完之后是否到达指定温度，到达立刻发出终止信号
            if(currentTemp>=targetTemp)
            {
                stopServe();
                emit signalServeOver();
            }
        }
        else if (mode==SERVER_MODE_COLD)
        {
            if(currentTemp>targetTemp)
            {
                if(fanSpeed==0)
                {
                    this->currentTemp-=fan[0]/120;
                    GetFee(fan[0]);
                }
                else if(fanSpeed==1)
                {
                    this->currentTemp-=fan[1]/120;
                    GetFee(fan[1]);
                }
                else if(fanSpeed==2)
                {
                    this->currentTemp-=fan[2]/120;
                    GetFee(fan[2]);
                }
                emit signalSendInfo();
            }
            // 判断减完之后是否到达指定温度，到达立刻发出终止信号
            if(currentTemp<=targetTemp)
            {
                stopServe();
                emit signalServeOver();
            }
        }
    });
}

bool Server::startServe()
{
    if(!durationTimer->isActive())
    {
        if(mode==SERVER_MODE_WARM)
        {
            if(currentTemp>=targetTemp)
            {
                state=SERVER_STATE_FREE;
                emit signalServeOver();
            }
            else
            {
                durationTimer->start();
                state=SERVER_STATE_SERVE;
            }
        }
        else if (mode==SERVER_MODE_COLD)
        {
            if(currentTemp<=targetTemp)
            {
                state=SERVER_STATE_FREE;
                emit signalServeOver();
            }
            else
            {
                durationTimer->start();
                state=SERVER_STATE_SERVE;
            }
        }
        return true;
    }
    else
    {
        QMessageBox::warning(this,"服务对象启动服务错误","服务对象已经在运行中");
        qDebug()<<"startServe:服务对象已经在运行中,serverId="<<serverId;
        return false;
    }
}

bool Server::stopServe()
{
    if(durationTimer->isActive())
    {
        durationTimer->stop();
        state=SERVER_STATE_FREE;
        return true;
    }
    else
    {
        QMessageBox::warning(this,"服务对象关闭服务错误","服务对象未在运行中");
        qDebug()<<"stopServe:服务对象未在运行中,serverId="<<serverId;
        return false;
    }
}

// REQUEST_ON时，分配给一个Server
bool Server::InitRoomServer(int roomId)
{
    this->roomId = roomId;
    this->state =SERVER_STATE_READY;
    return true;
}

//设定Server信息
void Server::SetServer(int roomId, int originTemp, int targetTemp,float currentTemp,int fanSpeed,float fee,float duration)
{
    this->roomId = roomId;
    this->targetTemp = targetTemp;
    this->fanSpeed=fanSpeed;
    this->fee = fee;
    this->duration=duration;
    this->originTemp=originTemp;
    this->currentTemp=currentTemp;
}

// SERVE状态
bool Server::SetRoom(float currentTemp)
{
    this->currentTemp=currentTemp;
    this->originTemp=currentTemp;
    this->state=SERVER_STATE_SERVE;
    return true;
}

// 修改已经分配到Server的Room的目标温度
bool Server::SChangeTargetTemp(int roomId, int targetTemp)
{
    // 如果修改的roomID和当前Server服务的roomID相同，修改目标温度
    if(this->roomId == roomId)
    {
        this->targetTemp = targetTemp;   // 修改目标温度
        return true;
    }
    else
    {
        qDebug()<<"服务对象ID与房间ID不对应";
        return false;
    }

}

// 修改风速
bool Server::SChangeFanSpeed(int roomId, int fanSpeed)
{
    // 如果修改的roomID和当前Server服务的roomID相同，修改风速
    if(this->roomId == roomId)
    {
        this->fanSpeed = fanSpeed;   // 修改风速
        return true;
    }
    else
    {
        qDebug()<<"服务对象ID与房间ID不对应";
        return false;
    }
}

bool Server::ChangeState(int state)
{
    this->state = state;
    return true;
}

bool Server::ChangeServerObj(Room room)
{
    this->state = SERVER_STATE_SERVE;
    // 新roomId在scheduler中已经被赋值
    this->roomId=room.roomId;
    this->fee=0;
    this->duration=0;
    this->originTemp=room.originTemp;
    this->currentTemp=room.currentTemp;
    this->targetTemp=room.targetTemp;
    this->fanSpeed=room.currentFanSpeed;
    return true;
}

void Server::GetFee(float fan)
{
    fee+=CalcFee(fan)/120;
}

// 计算每分钟费用，fan为每分钟温度变化
float Server::CalcFee(float fan)
{
    return feeRate*fan;
}

// 设置服务对象参数
void Server::SetACPara(int mode,int defaultTargetTemp,int defaultFanSpeed,float feeRate,float fan[])
{
    this->roomId = -1;  // 无绑定房间
    this->state = SERVER_STATE_FREE; // 空闲
    this->mode = mode;    // 默认制冷
    this->targetTemp = 22;    // 默认22度
    this->currentTemp=22;
    this->defaultTargetTemp=defaultTargetTemp;
    this->defaultFanSpeed=defaultFanSpeed;
    this->fanSpeed = 1;       // 默认低风速
    this->feeRate = feeRate;        // 费率默认1.0
    this->fee = 0;    // 费用默认0
    this->fan[0]=fan[0];
    this->fan[1]=fan[1];
    this->fan[2]=fan[2];
    this->duration=0;
}

bool Server::SetState(int state)
{
    this->state = state;   // SERVER_STATE_READY 1    // 服务开，但没有room
    return true;
}
