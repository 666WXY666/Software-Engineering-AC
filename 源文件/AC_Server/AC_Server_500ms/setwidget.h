#ifndef SETWIDGET_H
#define SETWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QString>
#include <QMessageBox>
#include <QDebug>

namespace Ui {
class SetWidget;
}

class SetWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SetWidget(QWidget *parent = nullptr);
    ~SetWidget();

    int mode;// 空调模式，0是制冷1是制热
    int defaultFanSpeed;// 默认风速
    int defaultTemp;// 默认温度
    int tempHighLimit;// 空调温度上限
    int tempLowLimit;// 空调温度下限
    float feeRate;// 空调费率（低）
    float fanSpeed[3];


    void Refreash();// 用于刷新界面

signals:
    void SetBacktoWidget();// 信号，用于返回主页面

private:
    Ui::SetWidget *ui;


private slots:
    //void SettoWidget();// 槽函数，用于返回主页面
    void GetPara();//槽函数，用于获取管理员设置的服务器参数



};

#endif // SETWIDGET_H
