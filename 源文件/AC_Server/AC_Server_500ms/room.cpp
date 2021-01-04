#include "room.h"

Room::Room(QWidget *parent) :
    QWidget(parent)
{
    waitTimeTimer=new QTimer(this);
    waitTimeTimer->setInterval(1000);
    connect(waitTimeTimer,&QTimer::timeout,[=](){
        waitServerTime++;
    });
}

Room::Room(const Room& room)
{
    this->roomId=room.roomId;// 房间ID
    this->serverId=room.serverId;        // ServerID
    this->waitId=room.waitId;//等待队列ID
    this->originTemp=room.originTemp;// 初始温度
    this->currentTemp=room.currentTemp;// 当前温度
    this->currentFanSpeed=room.currentFanSpeed;// 当前风速
    this->targetTemp=room.targetTemp;// 目标温度
    this->waitServerTime=room.waitServerTime;   //等待服务时长
    this->serverTime=room.serverTime;     //服务时长
    this->feeRate=room.feeRate;// 费率
    this->fee=room.fee;// 费用
    this->state=room.state;// 房间状态

    waitTimeTimer=new QTimer(this);
    waitTimeTimer->setInterval(1000);
    connect(waitTimeTimer,&QTimer::timeout,[=](){
        waitServerTime++;
    });
}

int Room::GetRoomId(void)
{
    return this->roomId;
}


void Room::ChangeTargetTemp(int targetTemp)
{
    this->targetTemp=targetTemp;
}
void Room::ChangeFanSpeed(int fanSpeed)
{
    this->currentFanSpeed=fanSpeed;
}
void Room::SetServerId(int serverId)
{
    this->serverId=serverId;
}


void Room::SetWaitQueueRoom(int roomId,int fanSpeed)
{
    this->roomId=roomId;
    this->currentFanSpeed=fanSpeed;
    this->waitServerTime=0;
    //分配一个等待时长
}
void Room::SetServerQueueRoom(int roomId,int serverId,int fanSpeed)
{
    this->roomId=roomId;
    this->serverId=serverId;
    this->currentFanSpeed=fanSpeed;
    this->serverTime=0;
}
int Room::GetWaitServerTime(void)
{
    return this->waitServerTime;
}
void Room::ChangeServerState(int newState)
{
    state=newState;
}

void Room::SetWaitTime(int waitTime)
{
    this->waitServerTime=waitTime;
}

void Room::ChangeServer(int state)
{
    this->state=state;
}
