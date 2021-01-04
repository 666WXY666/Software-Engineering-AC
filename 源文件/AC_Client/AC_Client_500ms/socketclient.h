#ifndef SOCKETCLIENT_H
#define SOCKETCLIENT_H

#include <QWidget>
#include <QTcpSocket>
#include <QHostAddress>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>

// 客户端请求码
#define REQUEST_ON 100// 空调开启
#define REQUEST_CHANGE_TEMP 200// 修改温度
#define REQUEST_CHANGE_FAN_SPEED 300// 修改风速
#define REQUEST_OFF 400// 空调关闭
// 客户端通知码
#define REQUEST_REWORM_TEMP 000// 回温温度
// 服务器端通知码
#define REQUEST_INFO 000// 当前温度和总费用心跳包
#define REQUEST_SERVE_ON 500// 服务启动
#define REQUEST_SERVE_OVER 600// 服务完成
#define REQUEST_SERVE_PREEMPT 700// 服务暂停（被抢占或者请求开启服务没有满足）
// 服务器端应答码
#define ACK_ON_OK 101// 空调开启-OK
#define ACK_ON_ERROR 102// 空调开启-ERROR
#define ACK_CHANGE_TEMP_OK 201// 修改温度-OK
#define ACK_CHANGE_TEMP_ERROR 202// 修改温度-ERROR
#define ACK_CHANGE_FAN_SPEED_OK 301// 修改风速-OK
#define ACK_CHANGE_FAN_SPEED_ERROR 302// 修改风速-ERROR
#define ACK_OFF_OK 401// 关闭-OK
#define ACK_OFF_ERROR 402// 关闭-ERROR
// 客户端应答码
#define ACK_SERVE_ON_OK 501// 服务启动-OK
#define ACK_SERVE_ON_ERROR 502// 服务启动-ERROR
#define ACK_SERVE_OVER_OK 601// 服务完成-OK
#define ACK_SERVE_OVER_ERROR 602// 服务完成-ERROR
#define ACK_SERVE_PREEMPT_OK 701// 服务暂停（被抢占或者请求开启服务没有满足）-OK
#define ACK_SERVE_PREEMPT_ERROR 702// 服务暂停（被抢占或者请求开启服务没有满足）-ERROR
// 客户端状态
#define STATE_ON 0// 刚收到开机请求，初始化的默认状态
#define STATE_SERVE 1// 在服务队列接受服务
#define STATE_WAIT 2// 在等待队列等待
#define STATE_HANG_UP 3// 已经达到指定温度，但没有关机，且在等待队列回温
#define STATE_OFF 4// 关机（实际没什么意义，因为此时room已经被销毁了）
// 模式
#define SERVER_MODE_COLD 0      // 制冷 default
#define SERVER_MODE_WARM 1      // 制热
class SocketClient : public QWidget
{
    Q_OBJECT
public:
    QTcpSocket *tcpSocket=nullptr;// 通信套接字

    explicit SocketClient(QWidget *parent = nullptr);
    ~SocketClient();

    void JsonAnalyse(QJsonObject receivedJson);

    int defaultTemp=22;// 默认温度
    int defaultFanSpeed=1;// 默认风速

    float totalFee=0;// 总费用
    float currentTemp;// 当前温度

    int serverId=-1;// 服务对象ID
    int waitId=0;// 等待队列ID
    int waitTime=0;// 等待时间

    int originTemp;// 初始室温

    int mode=1;

    int state=STATE_OFF;// 客户端状态

    void SendRequestOn(int roomId);// 发送开机请求包
    void SendRequestChangeTargetTemp(int roomId,int targetTemp);// 发送修改温度请求包
    void SendRequestChangeFanSpeed(int roomId,int fanSpeed);// 发送修改风速请求包
    void SendRequestOff(int roomId);// 发送关机请求包
    void SendRewormTemp();// 回温温度心跳包

signals:
    void signalShowDefaultPara();// 收到了服务器发来的默认参数，温度和风速显示在界面上的目标温度和当前风速
    void signalTempFanButtonEnable();// 收到了服务器发来的开启服务或者服务不满足等待的包，设置修改温度和修改风速按钮可以点击
    void signalStartRewormProgrom();// 启动回温程序信号
    void signalChangeState();// 修改客户端当前状态的信号，只需将读取state，刷新显示
    void signalRefreshTempFee();// 收到了服务器发来的当前温度和费用，只需将读取currentTemp和totalFee，刷新显示
    void signalTcpDisconnected();// 与服务器的连接断开，停止一切动作，关闭空调，回到初始未开机的样子
    void signalTempChangeOk();
    void signalFanSpeedChangeOk();

private:

    // 槽函数
    void OnTcpClientConnected();// 连接成功
    void OnTcpClientDisconnected();// 断开连接


};

#endif // SOCKETCLIENT_H
