#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <QTime>
#include <QTimer>
#include <QList>
#include <QDebug>
#include <QMessageBox>
#include "room.h"

#define SERVER_MODE_COLD 0      // 制冷 default
#define SERVER_MODE_WARM 1      // 制热

#define SERVER_FAN_LOW 0      // 低速风
#define SERVER_FAN_MID 1      // 中速风
#define SERVER_FAN_HIGH 2      // 高速风

#define SERVER_STATE_READY 0// 收到了请求，初始化的默认状态，等待客户端发送当前温度，不能接受请求
#define SERVER_STATE_SERVE 1// 收到了客户端发送的当前温度，正在服务
#define SERVER_STATE_FREE 2// 空闲，可以接受请求

class Server : public QWidget
{
    Q_OBJECT

private:


public:
    // self info
    int serverId;    // 服务器ID
    int roomId=-1;      // 房间ID，空闲状态为-1

    int originTemp;      // 该server初始化时的温度，用于计算温度改变进而获取费用
    float currentTemp;     // 当前温度
    int targetTemp=22;      // 目标温度
    int fanSpeed=1;        // 当前风速

    float fee=0;              // 费用

    int state=SERVER_STATE_FREE;           // 状态
    int duration=0;         // 使用时长

    // 管理员：server info
    // SetACPara
    int mode;            // 模式,有：COLD-0/WARM-1
    int defaultTargetTemp;   // 默认目标温度
    int defaultFanSpeed;// 默认目标风速
    float feeRate;          // 费率
    float fan[3];   // 风速值,每分钟温度变化

    QTimer *durationTimer;
    QTimer *tempTimer;

    explicit Server(int serverId,int mode,int defaultTargetTemp,int defaultFanSpeed,float feeRate,float fan[],QWidget *parent = nullptr);

    int GetRoomId(void){return this->roomId;}
    // 设置Server的信息
    bool InitRoomServer(int roomId);
    void SetServer(int roomId, int originTemp, int targetTemp,float currentTemp,int fanSpeed,float fee,int duration);
    bool SetRoom(float currentTemp);
    // 修改目标温度、修改风速
    bool SChangeTargetTemp(int roomId, int targetTemp);
    bool SChangeFanSpeed(int roomId, int fanSpeed);

    bool ChangeState(int state);
    bool ChangeServerObj(Room room);
    void GetFee(float fan);
    float CalcFee(float fan);
    void SetACPara(int mode,int defaultTargetTemp,int defaultFanSpeed,float feeRate,float fan[]);
    bool SetState(int state);

    bool startServe();
    bool stopServe();

signals:
    void signalServeOver();// 到达指定温度后发出
    void signalSendInfo();// 温度变化后发出，发送给客户端心跳通知包
    void signalDurationRefresh();
};

#endif // SERVER_H
