#include "waitqueue.h"

WaitQueue::WaitQueue(int defaultTemp,int defaultFanSpeed,float feeRate,QWidget *parent) : QWidget(parent)
{
    Init(defaultTemp,defaultFanSpeed,feeRate);
}

bool WaitQueue::AddInWaitQueue(int roomId)
{
    // 创建一个房间并初始化
    Room *room=new Room;
    room->roomId=roomId;
    room->serverId=-1;
    room->waitId=1;
    room->waitServerTime=0;
    room->currentFanSpeed=defaultFanSpeed;// 当前风速
    room->targetTemp=defaultTemp;// 目标温度
    room->serverTime=0;     //服务时长
    room->feeRate=feeRate;// 费率
    room->fee=0;// 费用
    room->state=ROOM_STATE_ON;// 房间状态
    room->waitTimeTimer->start();
    // 将这个room加入到waitQueue中
    roomMap.insert(roomId,room);
    return true;
}

bool WaitQueue::AddInWaitQueue(Room *room)
{
    // 创建一个房间并初始化
    room->serverId=-1;        // ServerID
    room->waitId=1;// 等待队列ID
    room->waitServerTime=0;   // 等待服务时长
    room->serverTime=0;// 清空服务时长
    room->state=ROOM_STATE_READY;// 房间状态
    room->waitTimeTimer->start();
    // 将这个room加入到waitQueue中
    roomMap.insert(room->roomId,room);
    return true;
}

QList<int> WaitQueue::equalFanSpeedRoomId(int fanSpeed)// 返回等待队列中风速为fanSpeed的房间ID
{
    QList<int> equalFanSpeedRoomIdList;
    for (QMap<int,Room*>::iterator it=roomMap.begin();it!=roomMap.end();it++)
    {
        if(it.value()->currentFanSpeed==fanSpeed)
        {
            equalFanSpeedRoomIdList.append(it.key());
        }
    }
    return equalFanSpeedRoomIdList;
}

// 返回风速高于fanSpeed的房间ID
QList<int> WaitQueue::higherFanSpeed(int fanSpeed)
{
    QList<int> RoomIdList;
    for (QMap<int,Room*>::iterator it=roomMap.begin();it!=roomMap.end();it++)
    {
        if(it.value()->currentFanSpeed>fanSpeed)
        {
            RoomIdList.append(it.key());
        }
    }
    return RoomIdList;
}

// 返回风速最高的fanSpeed的房间ID，如果有多个返回等待时间最长的，如果waitqueue为空，返回-1
int WaitQueue::GetMaxWaitTime()
{
    if(roomMap.size()>0)
    {
        QList<int> highestFanSpeedRoomIdList;
        int highestFanSpeed=roomMap.begin().value()->currentFanSpeed;
        highestFanSpeedRoomIdList.append(roomMap.begin().key());
        for (QMap<int,Room*>::iterator it=roomMap.begin();it!=roomMap.end();it++)
        {
            if(it!=roomMap.begin())
            {
                if(it.value()->currentFanSpeed>highestFanSpeed)
                {
                    highestFanSpeed=it.value()->currentFanSpeed;
                    highestFanSpeedRoomIdList.clear();
                    highestFanSpeedRoomIdList.append(it.key());
                }
                else if(it.value()->currentFanSpeed==highestFanSpeed)
                {
                    highestFanSpeedRoomIdList.append(it.key());
                }
            }
        }
        int maxWaitTime=roomMap[highestFanSpeedRoomIdList[0]]->waitServerTime;
        int maxRoomId=highestFanSpeedRoomIdList[0];
        for (int i=1;i<highestFanSpeedRoomIdList.size();i++)
        {
            if(roomMap[highestFanSpeedRoomIdList[i]]->waitServerTime>maxWaitTime)
            {
                maxWaitTime=roomMap[highestFanSpeedRoomIdList[i]]->waitServerTime;
                maxRoomId=highestFanSpeedRoomIdList[i];
            }
        }
        return maxRoomId;
    }
    else
    {
        return -1;
    }
}

bool WaitQueue::MoveOutWaitQueue(int roomId)
{
    if(roomMap.keys().contains(roomId))
    {
        roomMap[roomId]->waitTimeTimer->stop();
        roomMap.remove(roomId);
        return true;
    }
    else
    {
        return false;
    }
}

bool WaitQueue::QChangeTargetTemp(int roomId, int targetTemp)
{
    for (QMap<int,Room*>::iterator it=roomMap.begin();it!=roomMap.end();it++)
    {
        if(it.key()==roomId)
        {
            it.value()->targetTemp=targetTemp;
            return true;
        }
    }
    QMessageBox::warning(this,"修改目标温度错误","等待队列未找到该房间");
    qDebug()<<"QChangeTargetTemp:等待队列未找到该房间";
    return false;
}

bool WaitQueue::QChangeFanSpeed(int roomId, int fanSpeed)
{
    for (QMap<int,Room*>::iterator it=roomMap.begin();it!=roomMap.end();it++)
    {
        if(it.key()==roomId)
        {
            it.value()->currentFanSpeed=fanSpeed;
            return true;
        }
    }
    QMessageBox::warning(this,"修改目标风速错误","等待队列未找到该房间");
    qDebug()<<"QChangeFanSpeed:等待队列未找到该房间";
    return false;
}

void WaitQueue::Init(int defaultTemp,int defaultFanSpeed,float feeRate)
{
    this->currentSize=0;
    this->waitQueueSize=10;//暂设最大等待队列为10
    this->defaultTemp=defaultTemp;
    this->defaultFanSpeed=defaultFanSpeed;
    this->feeRate=feeRate;
}
