#ifndef SERVERWIDGET_H
#define SERVERWIDGET_H

#include <QWidget>
#include "setwidget.h"
#include "monitorwidget.h"
#include "receptionwidget.h"
#include "managerwidget.h"
#include "scheduler.h"
#include "statistic.h"
#include "dao.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ServerWidget; }
QT_END_NAMESPACE

class ServerWidget : public QWidget
{
    Q_OBJECT

public:
    ServerWidget(QWidget *parent = nullptr,Scheduler *scheduler = nullptr);
    ~ServerWidget();

signals:

private:
    Ui::ServerWidget *ui;
    MonitorWidget M_Widget;// 监视窗口
    SetWidget S_Widget;// 设置窗口
    ReceptionWidget R_Widget;// 前台业务窗口
    ManagerWidget Ma_Widget;// 经理业务窗口

    Scheduler *scheduler;
    Statistic *statistic;
    DAO *dao;

    void InitScheduler();// 初始化网络
    void InitDAO();// 初始化数据库
    void Init();

private slots:

    void PowerOn();//开启主界面
    void toSet();// 槽函数，用于进入设置窗口
    void toMonitor();// 槽函数，用于进入监视窗口
    void toReception();// 槽函数，用于进入前台页面
    void toManager();// 槽函数，用于进入经理页面
    void DealSetSig();//槽函数，处理设置窗口发送的信号，用于返回主界面
    void DealMonitorSig();// 槽函数，处理监视窗口发送的信号，用于返回主界面
    void DealReceptionSig();// 槽函数，处理前台窗口发出的信号，用于返回主界面
    void DealManagerSig();// 槽函数，处理经理页面发出的信号，用于返回主界面
    void Close();// 停止服务器端

};
#endif // SERVERWIDGET_H
