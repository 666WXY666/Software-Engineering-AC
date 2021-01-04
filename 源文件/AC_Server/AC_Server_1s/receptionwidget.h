#ifndef RECEPTIONWIDGET_H
#define RECEPTIONWIDGET_H

#include <QWidget>
#include "dao.h"
#include "log.h"
#include <fstream>
#include <iostream>
#include <string>
#include <QList>
#include "statistic.h"


using namespace std;

namespace Ui {
class ReceptionWidget;
}

class ReceptionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ReceptionWidget(QWidget *parent = nullptr);
    ~ReceptionWidget();

    Statistic *statistic;

signals:
    void ReceptionBacktoWidget();// 信号，用于返回主界面

private:
    Ui::ReceptionWidget *ui;

private slots:
    void ReceptiontoWidget();// 槽函数，用于返回主界面
    void PrintBills();// 打印账单
    void PrintDetails();// 打印详单
};



#endif // RECEPTIONWIDGET_H
