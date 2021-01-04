#include "setwidget.h"
#include "ui_setwidget.h"

SetWidget::SetWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SetWidget)
{
    ui->setupUi(this);

    // 初始化数值，便于检测
    mode=1;
    tempLowLimit=18;
    tempHighLimit=25;
    fanSpeed[0]=0.33;
    fanSpeed[1]=0.5;
    fanSpeed[2]=1;
    defaultFanSpeed=1;
    defaultTemp=22;
    feeRate=1;

    // 初始化界面
    ui->ModecomboBox->setCurrentIndex(mode);
    ui->TempHighEdit->setText(QString::number(tempHighLimit));
    ui->TempLowEdit->setText(QString::number(tempLowLimit));
    ui->FeeLEdit->setText(QString::number(feeRate));
    ui->DefaultTempEdit->setText(QString::number(defaultTemp));
    ui->DefaultFanBox->setCurrentIndex(defaultFanSpeed);
    ui->FanLEdit->setText(QString::number(fanSpeed[0]));
    ui->FanMEdit->setText(QString::number(fanSpeed[1]));
    ui->FanHEdit->setText(QString::number(fanSpeed[2]));

    connect(ui->SetConfirmButton,&QPushButton::clicked,this,&SetWidget::GetPara);// 按下确认按钮获取输入的参数
}


SetWidget::~SetWidget()
{
    delete ui;
}

////////////////////////////////////////////////////////////// 以下为正常函数 ////////////////////////////////////////////////////////////////////

void SetWidget::Refreash()// 刷新界面
{
    ui->ModecomboBox->setCurrentText(0);
    ui->TempHighEdit->setText(QString::number(tempHighLimit));
    ui->TempLowEdit->setText(QString::number(tempLowLimit));
    ui->FeeLEdit->setText(QString::number(feeRate));
    ui->DefaultTempEdit->setText(QString::number(defaultTemp));
    ui->DefaultFanBox->setCurrentIndex(defaultFanSpeed);
    ui->FanLEdit->setText(QString::number(fanSpeed[0]));
    ui->FanMEdit->setText(QString::number(fanSpeed[1]));
    ui->FanHEdit->setText(QString::number(fanSpeed[2]));
    return ;
}


////////////////////////////////////////////////////////////// 以下为槽函数 ////////////////////////////////////////////////////////////////////

//void SetWidget::SettoWidget()// 发送要回到主界面的信号
//{

//    return;
//}

void SetWidget::GetPara()// 获取管理员设置的参数
{
    //获取工作模式
    int mode_T=ui->ModecomboBox->currentIndex();
    // 获取温度参数
    int h_Temp=ui->TempHighEdit->text().toInt();
    int l_Temp=ui->TempLowEdit->text().toInt();
    int temp_D=ui->DefaultTempEdit->text().toInt();
    float fee_T=ui->FeeLEdit->text().toFloat();
    float f_0=ui->FanLEdit->text().toFloat();
    float f_1=ui->FanMEdit->text().toFloat();
    float f_2=ui->FanHEdit->text().toFloat();
    float f_D=ui->DefaultFanBox->currentIndex();


    if(h_Temp<l_Temp){
        QMessageBox::critical(NULL, "critical", "温度参数错误 ！", QMessageBox::Yes);
        return;
    }

    if(temp_D<l_Temp||temp_D>h_Temp)
    {
        QMessageBox::critical(NULL, "critical", "非法默认温度 ！", QMessageBox::Yes);
        return;
    }

    if(f_0>f_1||f_1>f_2||f_0>f_2||f_0<0||f_1<0||f_2<0){
        QMessageBox::critical(NULL, "critical", "风速参数错误 ！", QMessageBox::Yes);
        return;
    }


    mode=mode_T;
    tempLowLimit=l_Temp;
    tempHighLimit=h_Temp;
    defaultTemp=temp_D;
    feeRate=fee_T;
    fanSpeed[0]=f_0;
    fanSpeed[1]=f_1;
    fanSpeed[2]=f_2;
    defaultFanSpeed=f_D;

    emit SetBacktoWidget();
    return;
}



