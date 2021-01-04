#include "serverqueue.h"

ServerQueue::ServerQueue(int defaultTemp,int defaultFanSpeed,float feeRate,QWidget *parent) : QWidget(parent)
{
    Init(defaultTemp,defaultFanSpeed,feeRate);
}

bool ServerQueue::AddInServerQueue(int roomId,int serverId)
{
    // 创建一个房间并初始化
    Room *room=new Room(this);
    room->roomId=roomId;
    room->serverId=serverId;
    room->waitId=0;
    room->waitServerTime=0;
    room->currentFanSpeed=defaultFanSpeed;// 当前风速
    room->targetTemp=defaultTemp;// 目标温度
    room->serverTime=0;     //服务时长
    room->feeRate=feeRate;// 费率
    room->fee=0;// 费用
    room->state=ROOM_STATE_ON;// 房间状态
    // 将这个room加入到serverQueue中
    roomMap.insert(roomId,room);
    return true;
}

bool ServerQueue::AddInServerQueue(Room *room,int serverId)
{
    room->serverId=serverId;        // ServerID
    room->waitId=0;// 等待队列ID
    room->waitServerTime=0;   // 等待服务时长
    room->serverTime=0;// 清空服务时长
    room->state=ROOM_STATE_READY;// 房间状态
    // 将这个room加入到serverQueue中
    roomMap.insert(room->roomId,room);
    return true;
}

// 返回风速低于fanSpeed的服务对象的房间ID
QList<int> ServerQueue::GetServerLowerFanSpeed(int fanSpeed)
{
    QList<int> RoomIdList;
    for (QMap<int,Room*>::iterator it=roomMap.begin();it!=roomMap.end();it++)
    {
        if(it.value()->currentFanSpeed<fanSpeed)
        {
            RoomIdList.append(it.key());
        }
    }
    return RoomIdList;
}

// 返回风速高于fanSpeed的服务对象的房间ID
QList<int> ServerQueue::GetServerHigherFanSpeed(int fanSpeed)
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

// 返回风速等于fanSpeed的服务对象的房间ID
QList<int> ServerQueue::GetServerEqualFanSpeed(int fanSpeed)
{
    QList<int> RoomIdList;
    for (QMap<int,Room*>::iterator it=roomMap.begin();it!=roomMap.end();it++)
    {
        if(it.value()->currentFanSpeed==fanSpeed)
        {
            RoomIdList.append(it.key());
        }
    }
    return RoomIdList;
}

// 返回所有服务对象中风速最小且服务时间最长的服务对象的房间ID，服务队列为空返回-1
float ServerQueue::GetServerLowestFanSpeedLongestServeTime()
{
    if(roomMap.size()>0)
    {
        // 获取最小风速房间ID列表
        int lowestFanSpeed=roomMap.begin().value()->currentFanSpeed;
        QList<int> lowestFanSpeedRoomIdList;
        lowestFanSpeedRoomIdList.append(roomMap.begin().key());
        for (QMap<int,Room*>::iterator it=roomMap.begin();it!=roomMap.end();it++)
        {
            if(it!=roomMap.begin())
            {
                if(lowestFanSpeed>it.value()->currentFanSpeed)
                {
                    lowestFanSpeed=it.value()->currentFanSpeed;
                    lowestFanSpeedRoomIdList.clear();
                    lowestFanSpeedRoomIdList.append(it.key());
                }
                else if (lowestFanSpeed==it.value()->currentFanSpeed)
                {
                    lowestFanSpeedRoomIdList.append(it.key());
                }
            }
        }
        // 在最小风速房间中找出服务时长最大的房间
        float longestServeTime=roomMap[lowestFanSpeedRoomIdList[0]]->serverTime;
        int longestServeTimeRoomId=lowestFanSpeedRoomIdList[0];
        for (int i=1;i<lowestFanSpeedRoomIdList.size();i++)
        {
            if(longestServeTime<roomMap[lowestFanSpeedRoomIdList[i]]->serverTime)
            {
                longestServeTime=roomMap[lowestFanSpeedRoomIdList[i]]->serverTime;
                longestServeTimeRoomId=lowestFanSpeedRoomIdList[i];
            }
        }
        return longestServeTimeRoomId;
    }
    else
    {
        return -1;
    }
}

QList<int> ServerQueue::GetServerInfo()
{
    return roomMap.keys();
}

bool ServerQueue::SetServerObj(int serverId, int roomId)
{
    roomMap[roomId]->serverId=serverId;
    return true;
}

bool ServerQueue::MoveOutServerQueue(int roomId)
{
    if(roomMap.remove(roomId)==1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void ServerQueue::Init(int defaultTemp,int defaultFanSpeed,float feeRate)
{
    this->serverQueueSize=3;
    this->currentSize=0;
    this->defaultTemp=defaultTemp;
    this->defaultFanSpeed=defaultFanSpeed;
    this->feeRate=feeRate;
}
