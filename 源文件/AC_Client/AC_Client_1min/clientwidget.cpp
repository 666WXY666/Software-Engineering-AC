#include "clientwidget.h"
#include "ui_clientwidget.h"


ClientWidget::ClientWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ClientWidget)
{
    ui->setupUi(this);
	
	socketClient=new SocketClient(this);

    socketClient->originTemp=10;
    socketClient->currentTemp=10;
    ui->CurrentTempEdit->setText(QString::number(socketClient->originTemp));
    RoomID=0;
    ui->lineEdit->setText(QString::number(RoomID));

    bool ok;
    QString text = QInputDialog::getText(this, tr("初始温度输入"),tr("请输入房间初始温度"), QLineEdit::Normal,0, &ok);
    if (ok && !text.isEmpty())
    {
        socketClient->originTemp=text.toInt();
        socketClient->currentTemp=text.toInt();
        ui->CurrentTempEdit->setText(QString::number(socketClient->originTemp));
        qDebug("温度是：%d",socketClient->originTemp);
    }

    QString idText = QInputDialog::getText(this, tr("房间号输入"),tr("请输入房间的房间号"), QLineEdit::Normal,0, &ok);
    if (ok && !idText.isEmpty())
    {
        RoomID=idText.toInt();
        ui->lineEdit->setText(QString::number(RoomID));
        qDebug("ID是：%d",RoomID);
    }

    rewormProgromTimer=new QTimer(this);
    rewormProgromTimer->setInterval(60000);

    freshTimer=new QTimer(this);
    freshTimer->setInterval(100);
    connect(freshTimer,&QTimer::timeout,[=](){
        if((socketClient->mode==SERVER_MODE_COLD&&socketClient->currentTemp-SetedTemp>=1)||
           (socketClient->mode==SERVER_MODE_WARM&&SetedTemp-socketClient->currentTemp>=1))
        {
            // 停止回温
            stopRewormProgrom();
            // 重发请求
            socketClient->SendRequestOn(RoomID);
        }
    });

    socketClient->totalFee=0;// 初始费用为0

    // 在点开始按钮之前所有有关设置的控件不允许使用
    // 显示类控件除了温度编辑都不允许使用
    ui->CurrentFeeEdit->setDisabled(true);
    ui->CurrentTempEdit->setDisabled(true);
    ui->FanBox->setDisabled(true);
    ui->TempEdit->setDisabled(true);
    ui->lineEdit->setDisabled(true);
    ui->ChangeTempButton->setDisabled(true);
    ui->ChangeFanButton->setDisabled(true);
    ui->CurrentTempEdit->setText(QString::number(socketClient->currentTemp));
    ui->CurrentFeeEdit->setText(QString::number(socketClient->totalFee));
    ui->FanBox->setCurrentIndex(1);
    ui->lineEdit_FanSpeed->setText("中");
    ui->TempEdit->setText("22");
    ui->lineEdit_TargetTemp->setText("22");
    ui->lineEdit_2->setText("关机");
    ui->lineEdit_2->setDisabled(true);

    connect(ui->StartEndButton,&QPushButton::clicked,this,&ClientWidget::StartClient);
    // 点击开始按钮启动客户端
    connect(ui->ChangeFanButton,&QPushButton::clicked,this,&ClientWidget::GetSetFan);
    connect(ui->ChangeTempButton,&QPushButton::clicked,this,&ClientWidget::GetSetTemp);

    // 回温计时器timeout，温度变化
    connect(rewormProgromTimer,&QTimer::timeout,this,&ClientWidget::rewormProgrom);
    // 收到启动回温程序信号，启动回温计时器
    connect(socketClient,&SocketClient::signalStartRewormProgrom,this,&ClientWidget::startRewormProgrom);

    // 用于收到服务端返回的数据后温度的显示
    connect(socketClient,&SocketClient::signalShowDefaultPara,this,&ClientWidget::ShowDefaultPara);

    // 用于收到服务端返回的数据后费用的显示
    connect(socketClient,&SocketClient::signalRefreshTempFee,this,&ClientWidget::ShowCurrentPara);

    // 设置按钮可以按
    connect(socketClient,&SocketClient::signalTempFanButtonEnable,this,&ClientWidget::SetPuttonEnabled);

    //显示state
    connect(socketClient,&SocketClient::signalChangeState,this,&ClientWidget::ShowState);

    //断开连接
    connect(socketClient,&SocketClient::signalTcpDisconnected,this,&ClientWidget::DealDisConnected);

    connect(socketClient,&SocketClient::signalTempChangeOk,this,&ClientWidget::DealTempChangeOk);

    connect(socketClient,&SocketClient::signalFanSpeedChangeOk,this,&ClientWidget::DealFanSpeedChangeOk);

    connect(socketClient->tcpSocket,&QTcpSocket::connected,[=](){
        socketClient->SendRequestOn(RoomID);
        // 与设置有关的控件允许使用
        ui->StartEndButton->setText("关闭");// 将开始按钮的文字改为关闭
        ui->lineEdit_2->setText("开机");
        ui->FanBox->setDisabled(false);
        ui->ChangeFanButton->setDisabled(false);
        ui->TempEdit->setDisabled(false);
        ui->ChangeTempButton->setDisabled(false);
    });
}

void ClientWidget::DealTempChangeOk()
{
    SetedTemp=tempSetedTemp;
    ui->lineEdit_TargetTemp->setText(QString::number(SetedTemp));
}

void ClientWidget::DealFanSpeedChangeOk()
{
    Fan=tempFan;
    if(Fan==0)
        ui->lineEdit_FanSpeed->setText("低");
    else if(Fan==1)
        ui->lineEdit_FanSpeed->setText("中");
    else if(Fan==2)
        ui->lineEdit_FanSpeed->setText("高");
}

void ClientWidget::rewormProgrom()
{
    if(socketClient->currentTemp<socketClient->originTemp)
    {
        socketClient->currentTemp+=0.5;
    }
    else if (socketClient->currentTemp>socketClient->originTemp)
    {
        socketClient->currentTemp-=0.5;
    }
    // 刷新显示
    ui->CurrentTempEdit->setText(QString::number(socketClient->currentTemp));
    // 发送回温温度包
    socketClient->SendRewormTemp();
}

void ClientWidget::startRewormProgrom()// 启动计时器，启动回温程序
{
    if(!rewormProgromTimer->isActive())
    {
        rewormProgromTimer->start();
        freshTimer->start();
        qDebug()<<"回温计时器启动成功";
    }
    else
    {
        QMessageBox::warning(this,"警告","回温程序已经在运行中");
        qDebug()<<"回温程序已经在运行中";
    }
}

void ClientWidget::stopRewormProgrom()// 关闭计时器，关闭回温程序
{
    if(rewormProgromTimer->isActive())
    {
        rewormProgromTimer->stop();
        freshTimer->stop();
        qDebug()<<"回温计时器关闭成功";
    }
    else
    {
        QMessageBox::warning(this,"警告","回温程序还未开启，无法关闭");
        qDebug()<<"回温程序还未开启，无法关闭";
    }
}

ClientWidget::~ClientWidget()
{
    delete ui;
}

void ClientWidget::StartClient()
{
    if(ui->StartEndButton->text()=="打开")
    {
        socketClient->tcpSocket->connectToHost(QHostAddress("127.0.0.1"),8888);
        qDebug()<<"客户端连接服务器……";
    }
    else
    {
        socketClient->SendRequestOff(RoomID);
        ui->StartEndButton->setText("打开");
        ui->lineEdit_2->setText("关机");
        ui->FanBox->setDisabled(true);
        ui->TempEdit->setDisabled(true);
        ui->ChangeFanButton->setDisabled(true);
        ui->ChangeTempButton->setDisabled(true);
    }
}

void ClientWidget::GetSetTemp()
{
    if((socketClient->mode==SERVER_MODE_COLD&&ui->TempEdit->text().toInt()<=socketClient->currentTemp)||
       (socketClient->mode==SERVER_MODE_WARM&&ui->TempEdit->text().toInt()>=socketClient->currentTemp))
    {
        tempSetedTemp=ui->TempEdit->text().toInt();
        socketClient->SendRequestChangeTargetTemp(RoomID,tempSetedTemp);
    }
    // 温度设置不合理
    else
    {
        QMessageBox::warning(this,"温度设置不合理","请检查温度设置是否合理");
        qDebug()<<"温度设置不合理";
    }
}

void ClientWidget::GetSetFan()
{
    tempFan=ui->FanBox->currentIndex();
    socketClient->SendRequestChangeFanSpeed(RoomID,tempFan);
}

void ClientWidget::ShowDefaultPara()
{
    // 初始温度设置不合理，重新设置
    while((socketClient->mode==SERVER_MODE_COLD&&socketClient->defaultTemp>socketClient->originTemp)||
       (socketClient->mode==SERVER_MODE_WARM&&socketClient->defaultTemp<socketClient->originTemp))
    {
        bool ok;
        QString text = QInputDialog::getText(this, tr("初始温度设置不合理"),tr("请重新输入房间初始温度"), QLineEdit::Normal,0, &ok);
        if (ok && !text.isEmpty())
        {
            socketClient->originTemp=text.toInt();
            qDebug("温度是：%d",socketClient->originTemp);
        }
    }
    socketClient->currentTemp=socketClient->originTemp;
    socketClient->totalFee=0;
    SetedTemp=socketClient->defaultTemp;
    Fan=socketClient->defaultFanSpeed;
    ui->TempEdit->setText(QString::number(SetedTemp));
    ui->lineEdit_TargetTemp->setText(QString::number(SetedTemp));
    ui->FanBox->setCurrentIndex(Fan);
    if(Fan==0)
        ui->lineEdit_FanSpeed->setText("低");
    else if(Fan==1)
        ui->lineEdit_FanSpeed->setText("中");
    else if(Fan==2)
        ui->lineEdit_FanSpeed->setText("高");
    ShowCurrentPara();
}

void ClientWidget::ShowCurrentPara()
{
    ui->CurrentTempEdit->setText(QString::number(socketClient->currentTemp));
    ui->CurrentFeeEdit->setText(QString::number(socketClient->totalFee));
}

void ClientWidget::SetPuttonEnabled()
{
    ui->FanBox->setDisabled(false);
    ui->TempEdit->setDisabled(false);
}

void ClientWidget::ShowState()
{
    int state=socketClient->state;
    switch (state)
    {
    case STATE_ON:
        ui->lineEdit_2->setText("开机");
        break;
    case STATE_SERVE:
        ui->lineEdit_2->setText("服务中...");
        break;
    case STATE_WAIT:
        ui->lineEdit_2->setText("等待中...");
        break;
    case STATE_HANG_UP:
        ui->lineEdit_2->setText("回温中...");
        break;
    case STATE_OFF:
        ui->lineEdit_2->setText("关机");
        break;
    }
}

void ClientWidget::DealDisConnected()
{
    ui->CurrentFeeEdit->setDisabled(true);
    ui->CurrentTempEdit->setDisabled(true);
    ui->FanBox->setDisabled(true);
    ui->TempEdit->setDisabled(true);
    ui->lineEdit->setDisabled(true);
    ui->ChangeTempButton->setDisabled(true);
    ui->ChangeFanButton->setDisabled(true);
    ui->lineEdit_2->setText("关机");
    ui->lineEdit_2->setDisabled(true);
    ui->StartEndButton->setText("打开");
    if(rewormProgromTimer->isActive())
    {
        rewormProgromTimer->stop();
    }
    if(freshTimer->isActive())
    {
        freshTimer->stop();
    }
}
