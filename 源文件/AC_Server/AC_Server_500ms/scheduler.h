
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QString>
#include <QTimer>
#include <QMessageBox>
#include <QDebug>
#include <QMap>

#include "dao.h"
#include "server.h"
#include "serverqueue.h"
#include "waitqueue.h"

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

class Scheduler : public QWidget
{
    Q_OBJECT
public:
    explicit Scheduler(QWidget *parent = nullptr,DAO *dao=nullptr);
    ~Scheduler();

    int mode;// 模式
    int defaultTemp;// 默认温度
    int defaultFanSpeed;// 默认风速
    float fan[3];// 风速
    float feeRate;// 空调费率
    int tempHighLimit;// 空调温度上限
    int tempLowLimit;// 空调温度下限

    DAO *dao;

    // 声明一个QList，存3个Server
    QList<Server*> serverList;
    ServerQueue *serverQueue;
    WaitQueue *waitQueue;
    QMap<int,Room*> hangUpQueue;// 服务完成的房间存储在挂机队列

    QMap<int,Room*> allRoomMap;// 所有房间的map

    QTimer *RRTimer;// RR调度计时器

    void InitSocketServer();// 初始化
    void CloseServer();// 关闭服务器
    void init(int mode,int defaultTargetTemp,int defaultFanSpeed,float feeRate,float fan[3],int tempLowLimit,int tempHighLimit);
    int GetReplaceRoomId(int fanSpeed);// 返回>=0的数字，代表被替换的房间ID；返回==-1，该房间等待；
    void judgeRR();// 判断是否需要RR

    void sendServePreemptJson(int roomId);// 发送抢占通知包
    void sendServeOnJson(int roomId);// 发送服务开启通知包
    void sendServeOverJson(int roomId);// 发送服务完成通知包

    void sync(int roomId);// 在房间被移除服务队列之前，同步房间与服务对象信息，且将等待服务时长设为0



signals:

private:
    QTcpServer *tcpServer;// 监听套接字
    QMap<QTcpSocket*,int>tcpSockets;// 通信套接字map，value为房间ID
    QMap<int,QTcpSocket*>roomIds;// 房间IDmap，value为通信套接字

    void JsonAnalyse(QTcpSocket *tcpSocket,QJsonObject receivedJson);// 收到包，解包处理

    int RequestOn(int roomId,int fanSpeed,int flag);// 返回1在服务队列，返回0在等待队列，返回-1错误。flag为是否是重发的请求,1为新请求，0为重发请求
    bool RequestOff(int roomId);
    void RequestOver(int roomId);
    bool ChangeTargetTemp(int roomId,int targetTemp);
    bool ChangeFanSpeed(int roomId,int fanSpeed);

    // 槽函数
    void OnTcpServerConnected();// 有连接到来

};

#endif // SCHEDULER_H
