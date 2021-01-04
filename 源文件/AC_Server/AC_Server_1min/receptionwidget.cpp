#include "receptionwidget.h"
#include "ui_receptionwidget.h"

ReceptionWidget::ReceptionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReceptionWidget)
{
    ui->setupUi(this);

    connect(ui->BacktoWidgetButton,&QPushButton::clicked,this,&ReceptionWidget::ReceptiontoWidget);//按下按钮发送返回主界面的信号
    connect(ui->PrintBillButton,&QPushButton::clicked,this,&ReceptionWidget::PrintBills);// 按下按钮打印账单
    connect(ui->PrintDetailButton,&QPushButton::clicked,this,&ReceptionWidget::PrintDetails);// 按下按钮打印详单

}

ReceptionWidget::~ReceptionWidget()
{
    delete ui;
}


////////////////////////////////////////////////////////////// 以下为槽函数 ////////////////////////////////////////////////////////////////////

void ReceptionWidget::ReceptiontoWidget()// 槽函数，用于发送返回主界面的信号
{
    emit ReceptionBacktoWidget();
}

void ReceptionWidget::PrintBills()
{
    if(statistic->PrintBill(ui->RoomIdEdit->text().toInt()))
    {
        QMessageBox::information(this,"通知","生成账单成功");
        qDebug()<<"生成账单成功";
    }
    else
    {
        QMessageBox::warning(this,"警告","生成账单失败");
        qDebug()<<"生成账单失败";
    }
}

void ReceptionWidget::PrintDetails()
{
    if(statistic->PrintDetailList(ui->RoomIdEdit->text().toInt()))
    {
        QMessageBox::information(this,"通知","生成详单成功");
        qDebug()<<"生成账详单成功";
    }
    else
    {
        QMessageBox::warning(this,"警告","生成详单失败");
        qDebug()<<"生成详单失败";
    }
}
