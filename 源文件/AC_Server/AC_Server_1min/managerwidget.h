#ifndef MANAGERWIDGET_H
#define MANAGERWIDGET_H

#include <QWidget>
#include <fstream>
#include <iostream>
#include <string>
#include <QList>
#include "report.h"
#include "statistic.h"
#include <QChartView>
#include <QChart>
#include <QWidget>
#include <QtCharts>
QT_CHARTS_USE_NAMESPACE

namespace Ui {
class ManagerWidget;
}

class ManagerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ManagerWidget(QWidget *parent = nullptr);
    ~ManagerWidget();

    Statistic *statistic;

signals:
    void ManagerBacktoWidget();// 信号，用来返回主界面

private:
    Ui::ManagerWidget *ui;

private slots:
    void ManagertoWidget();// 槽函数，用来返回主界面
    void PrintReports();// 打印报表
    void ShowGraph();// 显示图形
};

#endif // MANAGERWIDGET_H
