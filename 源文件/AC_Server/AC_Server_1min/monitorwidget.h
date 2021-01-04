#ifndef MONITORWIDGET_H
#define MONITORWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QTimer>
#include "server.h"
#include "serverqueue.h"
#include "waitqueue.h"

namespace Ui {
class MonitorWidget;
}

class MonitorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MonitorWidget(QWidget *parent = nullptr);
    ~MonitorWidget();

    QMap<int,Room*> *allRoomMap;// 所有房间的map
    QTimer*timer;

    int mode;// 模式
    int defaultTemp;// 默认温度
    int defaultFanSpeed;// 默认风速
    float fan[3];// 风速
    float feeRate;// 空调费率
    int tempHighLimit;// 空调温度上限
    int tempLowLimit;// 空调温度下限

    void ShowRooms();
    void ShowDefaultPara();

signals:
    void MonitorBacktoWidget();// 槽函数，用于发送信号返回主界面

private:
    Ui::MonitorWidget *ui;

private slots:
    void MonitortoWidget();// 信号，用于返回主界面

};

#endif // MONITORWIDGET_H
