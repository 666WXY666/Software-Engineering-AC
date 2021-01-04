#ifndef ROOM_H
#define ROOM_H

#include <QObject>
#include <QWidget>
#include <QTimer>

#define ROOM_STATE_ON 0// 刚收到开机请求，初始化的默认状态
#define ROOM_STATE_READY 1// 已经成功开机，还未发送调度结果
#define ROOM_STATE_SERVE 2// 在服务队列接受服务
#define ROOM_STATE_WAIT 3// 在等待队列等待
#define ROOM_STATE_HANG_UP 4// 已经达到指定温度，但没有关机，且在挂机队列回温
#define ROOM_STATE_OFF 5// 关机（实际没什么意义，因为此时room已经被销毁了）

class Room : public QWidget
{
    Q_OBJECT
public:

    // self info
    int customerId=0;  // 顾客ID
    bool isUsing=true;       // 是否正在使用
    bool isBooked=true;      // 是否已被预订

    int roomId;// 房间ID
    int serverId=-1;        // ServerID
    int waitId=-1;//等待队列ID
    int originTemp=0;// 初始温度
    float currentTemp=0;// 当前温度
    int currentFanSpeed=1;// 当前风速
    int targetTemp=22;// 目标温度
    int waitServerTime=0;   //等待服务时长
    float serverTime=0;     //服务时长
    float feeRate=1;// 费率
    float fee=0;// 费用
    int state=ROOM_STATE_ON;// 房间状态

    QTimer *waitTimeTimer;

    explicit Room(QWidget *parent = nullptr);

    Room(const Room& room);

    int GetRoomId(void);
    void ChangeServerState(int newState);
    void SetWaitTime(int waitTime);
    void ChangeServer(int state);
    void RewarmProgram();
    void SetServerId(int serverId);
    void ChangeTargetTemp(int targetTemp);
    void ChangeFanSpeed(int fanSpeed);
    void SetWaitQueueRoom(int roomId,int fanSpeed);
    void SetServerQueueRoom(int roomId,int serverId,int fanSpeed);
    int GetWaitServerTime(void);

signals:

private:


};

#endif // ROOM_H
