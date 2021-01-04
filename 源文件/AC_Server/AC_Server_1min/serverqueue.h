#ifndef SERVERQUEUE_H
#define SERVERQUEUE_H

#include <QObject>
#include <QWidget>
#include <QMap>
#include <QList>
#include <QTimer>
#include <QDebug>

#include "room.h"

class ServerQueue : public QWidget
{
    Q_OBJECT

private:

public:
    int serverQueueSize;     // 服务队列最大程度 3
    int currentSize;         // 当前服务队列的长度

    int defaultTemp;// 默认温度
    int defaultFanSpeed;// 默认风速
    float feeRate;// 空调费率
    QMap<int,Room*> roomMap;

    explicit ServerQueue(int defaultTemp,int defaultFanSpeed,float feeRate,QWidget *parent = nullptr);

    QList<int> GetServerHigherFanSpeed(int fanSpeed);// 返回风速高于fanSpeed的服务对象的房间ID
    QList<int> GetServerLowerFanSpeed(int fanSpeed);// 返回风速低于fanSpeed的服务对象的房间ID
    QList<int> GetServerEqualFanSpeed(int fanSpeed);// 返回风速等于fanSpeed的服务对象的房间ID
    int GetServerLowestFanSpeedLongestServeTime();// 返回所有服务对象中风速最小且服务时间最长的服务对象的房间ID，服务队列为空返回-1
    bool SetServerObj(int serverId, int roomId);
    bool AddInServerQueue(int roomId,int serverId);
    bool AddInServerQueue(Room *room,int serverId);
    QList<int> GetServerInfo();
    bool MoveOutServerQueue(int roomId);  //增加了删除server
    void Init(int defaultTemp,int defaultFanSpeed,float feeRate);

signals:

};

#endif // SERVERQUEUE_H
