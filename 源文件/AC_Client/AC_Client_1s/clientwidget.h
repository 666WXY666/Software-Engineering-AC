#ifndef CLIENTWIDGET_H
#define CLIENTWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QDebug>
#include <QTimer>
#include <qinputdialog.h>
#include "socketclient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ClientWidget; }
QT_END_NAMESPACE

// 客户端状态
#define STATE_ON 0// 刚收到开机请求，初始化的默认状态
#define STATE_SERVE 1// 在服务队列接受服务
#define STATE_WAIT 2// 在等待队列等待
#define STATE_HANG_UP 3// 已经达到指定温度，但没有关机，且在等待队列回温
#define STATE_OFF 4// 关机（实际没什么意义，因为此时room已经被销毁了）

class ClientWidget : public QWidget
{
    Q_OBJECT

public:
    ClientWidget(QWidget *parent = nullptr);
    ~ClientWidget();

    QTimer *rewormProgromTimer;// 回温程序计时器
	QTimer *freshTimer;// 回温刷新计时器
	
    void rewormProgrom();// 回温程序
    void startRewormProgrom();// 启动计时器，启动回温程序
    void stopRewormProgrom();// 关闭计时器，关闭回温程序

private slots:
    void StartClient();// 初始化空调客户端界面
    void GetSetTemp();// 获取设置的温度
    void GetSetFan();// 获取设置的风速
    // 未实现的槽函数
    // void CloseClient();// 关闭客户端，停止空调服务
    void ShowDefaultPara();
    void ShowCurrentPara();
    void SetPuttonEnabled();
    void ShowState();
    void DealDisConnected();
    void DealTempChangeOk();
    void DealFanSpeedChangeOk();

private:
    Ui::ClientWidget *ui;

    int tempSetedTemp;// 顾客设置的温度临时变量
    int tempFan;// 顾客设置的风速,0是低风，1是中风，2是高风临时变量

    int SetedTemp;// 顾客设置的温度
    int Fan;// 顾客设置的风速,0是低风，1是中风，2是高风

    int RoomID=-1;

    SocketClient *socketClient;

};
#endif // CLIENTWIDGET_H
