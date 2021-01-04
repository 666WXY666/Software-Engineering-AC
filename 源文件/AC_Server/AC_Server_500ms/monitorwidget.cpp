#include "monitorwidget.h"
#include "ui_monitorwidget.h"

MonitorWidget::MonitorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MonitorWidget)
{
    ui->setupUi(this);

    connect(ui->MonitorBacktoWidgetButton,&QPushButton::clicked,this,&MonitorWidget::MonitortoWidget);

    timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,&MonitorWidget::ShowRooms);
}

MonitorWidget::~MonitorWidget()
{
    delete ui;
}



////////////////////////////////////////////////////////////// 以下为槽函数 ////////////////////////////////////////////////////////////////////



void MonitorWidget::MonitortoWidget()
{
    if(timer->isActive())
    {
        timer->stop();
    }
    emit MonitorBacktoWidget();
}

void MonitorWidget::ShowDefaultPara()
{
    if(mode==SERVER_MODE_COLD)
    {
        ui->mode_lineEdit->setText("制冷");
    }
    else if(mode==SERVER_MODE_WARM)
    {
        ui->mode_lineEdit->setText("制热");
    }
    ui->DefaultTempEdit->setText(QString::number(defaultTemp));
    switch (defaultFanSpeed)
    {
    case SERVER_FAN_LOW:
        ui->default_fan_lineEdit->setText("低");
        break;
    case SERVER_FAN_MID:
        ui->default_fan_lineEdit->setText("中");
        break;
    case SERVER_FAN_HIGH:
        ui->default_fan_lineEdit->setText("高");
        break;
    default:
        ui->default_fan_lineEdit->setText("未知");
        break;
    }
    ui->FanLEdit->setText(QString::number(fan[SERVER_FAN_LOW]));
    ui->FanMEdit->setText(QString::number(fan[SERVER_FAN_MID]));
    ui->FanHEdit->setText(QString::number(fan[SERVER_FAN_HIGH]));
    ui->FeeLEdit->setText(QString::number(feeRate));
    ui->TempLowEdit->setText(QString::number(tempLowLimit));
    ui->TempHighEdit->setText(QString::number(tempHighLimit));
}

void MonitorWidget::ShowRooms()
{
    QString temp = "房间ID\t初始温度\t当前温度\t目标温度\t风速\t总费用\t服务ID\t服务时长\t等待时长\t状态\n";
    temp += "---------------------------------------------------------------------------------------------------------\n";
    for (QMap<int,Room*>::iterator it=allRoomMap->begin();it!=allRoomMap->end();it++)
    {
        QString ID=QString::number(it.value()->roomId);
        QString originTemp=QString::number(it.value()->originTemp);
        QString currentTemp=QString::number(it.value()->currentTemp);
        int currentFanSpeed=it.value()->currentFanSpeed;
        QString currentFanSpeedStr;
        switch (currentFanSpeed)
        {
        case SERVER_FAN_LOW:
            currentFanSpeedStr="低";
            break;
        case SERVER_FAN_MID:
            currentFanSpeedStr="中";
            break;
        case SERVER_FAN_HIGH:
            currentFanSpeedStr="高";
            break;
        default:
            currentFanSpeedStr="未知";
            break;
        }
        QString targetTemp=QString::number(it.value()->targetTemp);
        QString fee=QString::number(it.value()->fee);
        QString serverId=QString::number(it.value()->serverId);
        QString duration=QString::number(int(it.value()->serverTime));
        QString waitTime=QString::number(it.value()->waitServerTime);
        int state=it.value()->state;
        QString stateStr;
        switch (state)
        {
        case ROOM_STATE_ON:
            stateStr="开机";
            break;
        case ROOM_STATE_READY:
            stateStr="就绪";
            break;
        case ROOM_STATE_SERVE:
            stateStr="服务中";
            break;
        case ROOM_STATE_WAIT:
            stateStr="等待中";
            break;
        case ROOM_STATE_HANG_UP:
            stateStr="回温中";
            break;
        case ROOM_STATE_OFF:
            stateStr="关机";
            break;
        default:
            stateStr="未知";
            break;
        }
        temp+=ID+"\t"+originTemp+"\t"+currentTemp+"\t"+targetTemp+"\t"+currentFanSpeedStr
                +"\t"+fee+"\t"+serverId+"\t"+duration+"\t"+waitTime+"\t"+stateStr+"\n";
    }
    ui->RoomBrowser->setText(temp);

}
