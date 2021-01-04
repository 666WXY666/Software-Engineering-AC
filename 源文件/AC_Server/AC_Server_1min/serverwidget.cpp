#include "serverwidget.h"
#include "ui_serverwidget.h"

ServerWidget::ServerWidget(QWidget *parent,Scheduler *scheduler)
    : QWidget(parent),
    ui(new Ui::ServerWidget),scheduler(scheduler)
{
    ui->setupUi(this);

    InitDAO();

    ui->toSetButton->setDisabled(true);
    ui->toMonitorButton->setDisabled(true);
    ui->toManagerButton->setDisabled(true);
    ui->toReceptionButton->setDisabled(true);
    ui->CloseServerButton->setDisabled(true);

    connect(ui->StartServerButton,&QPushButton::clicked,this,&ServerWidget::PowerOn);// 开启
    connect(ui->CloseServerButton,&QPushButton::clicked,this,&ServerWidget::Close);// 关闭

    connect(ui->toSetButton,&QPushButton::clicked,this,&ServerWidget::toSet);// 点击按钮进入设置窗口
    connect(ui->toMonitorButton,&QPushButton::clicked,this,&ServerWidget::toMonitor);// 点击按钮进入监视窗口
    connect(ui->toReceptionButton,&QPushButton::clicked,this,&ServerWidget::toReception);// 点击按钮进入前台服务
    connect(ui->toManagerButton,&QPushButton::clicked,this,&ServerWidget::toManager);// 点击按钮进入经理业务

    connect(&S_Widget,&SetWidget::SetBacktoWidget,this,&ServerWidget::DealSetSig);// 接收信号，返回主窗口
    connect(&M_Widget,&MonitorWidget::MonitorBacktoWidget,this,&ServerWidget::DealMonitorSig);// 接受信号，返回主窗口
    connect(&R_Widget,&ReceptionWidget::ReceptionBacktoWidget,this,&ServerWidget::DealReceptionSig);// 接受信号，返回主窗口
    connect(&Ma_Widget,&ManagerWidget::ManagerBacktoWidget,this,&ServerWidget::DealManagerSig);// 接受信号，返回主窗口

}

ServerWidget::~ServerWidget()
{
    delete ui;
}

void ServerWidget::InitScheduler()// 初始化Scheduler
{
    scheduler->deleteLater();
    qDebug()<<"创建Scheduler";
    scheduler=new Scheduler(this,dao);

    statistic=new Statistic(dao,QDateTime::currentDateTime());
    R_Widget.statistic=statistic;
    Ma_Widget.statistic=statistic;
}

void ServerWidget::InitDAO()// 初始化数据库
{
    qDebug()<<"创建并初始化DAO";
    dao=new DAO(this);
}

////////////////////////////////////////////////////////////// 以下为槽函数 ////////////////////////////////////////////////////////////////////

void ServerWidget::PowerOn()
{
    ui->StartServerButton->setDisabled(true);
    ui->toSetButton->setDisabled(false);
    ui->toManagerButton->setDisabled(true);
    ui->toReceptionButton->setDisabled(true);
    ui->CloseServerButton->setDisabled(false);
    InitScheduler();
}

void ServerWidget::toSet()
{
    ui->toMonitorButton->setDisabled(false);
    this->hide();
    S_Widget.Refreash();
    S_Widget.show();
}

void ServerWidget::toMonitor()
{
    // 服务队列和等待队列等参数传入
    M_Widget.allRoomMap=&scheduler->allRoomMap;
    if(!M_Widget.timer->isActive())
        M_Widget.timer->start(100);
    M_Widget.mode=scheduler->mode;// 模式
    M_Widget.defaultTemp=scheduler->defaultTemp;// 默认温度
    M_Widget.defaultFanSpeed=scheduler->defaultFanSpeed;// 默认风速
    M_Widget.fan[0]=scheduler->fan[0];// 风速
    M_Widget.fan[1]=scheduler->fan[1];// 风速
    M_Widget.fan[2]=scheduler->fan[2];// 风速
    M_Widget.feeRate=scheduler->feeRate;// 空调费率
    M_Widget.tempHighLimit=scheduler->tempHighLimit;// 空调温度上限
    M_Widget.tempLowLimit=scheduler->tempLowLimit;// 空调温度下限
    M_Widget.ShowRooms();
    M_Widget.ShowDefaultPara();
    this->hide();
    M_Widget.show();
}

void ServerWidget::toReception()
{
    this->hide();
    R_Widget.show();
}

void ServerWidget::toManager()
{
    this->hide();
    Ma_Widget.show();
}

void ServerWidget::DealSetSig()
{
    // 将设置的默认参数传给Scheduler
    scheduler->init(S_Widget.mode,S_Widget.defaultTemp,S_Widget.defaultFanSpeed,S_Widget.feeRate,S_Widget.fanSpeed,S_Widget.tempLowLimit,S_Widget.tempHighLimit);
    // 初始化Scheduler的scoket，开始监听连接请求
    scheduler->InitSocketServer();
    // UI操作
    this->show();
    ui->toSetButton->setDisabled(true);
    ui->StartServerButton->setDisabled(true);
    S_Widget.hide();
    qDebug()<<"模式为："<<S_Widget.mode;
    qDebug()<<"默认温度为："<<S_Widget.defaultTemp;
    qDebug()<<"最高温度为："<<S_Widget.tempHighLimit<<"最低温度为："<<S_Widget.tempLowLimit;
    qDebug()<<"默认风速为："<<S_Widget.defaultFanSpeed;
    qDebug()<<"风速为："<<"低="<<S_Widget.fanSpeed[0]<<"度/分钟，"<<"中="<<S_Widget.fanSpeed[1]<<"度/分钟，"<<"高="<<S_Widget.fanSpeed[2]<<"度/分钟";
    qDebug()<<"费率为："<<S_Widget.feeRate<<"度/元";
}

void ServerWidget::DealMonitorSig()
{
    this->show();
    M_Widget.hide();
}

void ServerWidget::DealReceptionSig()
{
    this->show();
    R_Widget.hide();
}

void ServerWidget::DealManagerSig()
{
    this->show();
    Ma_Widget.hide();
}

void ServerWidget::Close()
{
    ui->CloseServerButton->setDisabled(true);
    ui->StartServerButton->setDisabled(false);
    ui->toManagerButton->setDisabled(false);
    ui->toReceptionButton->setDisabled(false);
    ui->toSetButton->setDisabled(true);
    ui->toMonitorButton->setDisabled(true);
    // 关闭Socket监听
    scheduler->CloseServer();
}
