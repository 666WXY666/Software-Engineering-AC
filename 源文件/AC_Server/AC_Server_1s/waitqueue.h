#ifndef WAITQUEUE_H
#define WAITQUEUE_H

#include <QObject>
#include <QWidget>
#include <QVector>
#include <QTime>
#include <QDebug>
#include <QMessageBox>

#include "room.h"

class WaitQueue : public QWidget
{
    Q_OBJECT

private:

public:
    int waitQueueSize;   // 等待队列最大长度
    int currentSize;     // 当前等待队列长度

    int defaultTemp;// 默认温度
    int defaultFanSpeed;// 默认风速
    float feeRate;// 空调费率

    QMap<int,Room*> roomMap;

    explicit WaitQueue(int defaultTemp,int defaultFanSpeed,float feeRate,QWidget *parent = nullptr);

    bool AddInWaitQueue(int roomId);
    bool AddInWaitQueue(Room *room);
    bool MoveOutWaitQueue(int roomId);

    QList<int> equalFanSpeedRoomId(int fanSpeed);// 返回等待队列中风速为fanSpeed的房间ID
    QList<int> higherFanSpeed(int fanSpeed);// 返回风速高于fanSpeed的房间ID
    int GetMaxWaitTime();// 返回风速最高的fanSpeed的房间ID，如果有多个返回等待时间最长的，如果waitqueue为空，返回-1

    bool QChangeTargetTemp(int roomId, int targetTemp);
    bool QChangeFanSpeed(int roomId, int fanSpeed);
    void Init(int defaultTemp,int defaultFanSpeed,float feeRate);
signals:

};

#endif // WAITQUEUE_H
