#ifndef DAO_H
#define DAO_H

#include <QObject>
#include <QWidget>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QtDebug>
#include <QString>
#include <QList>
#include <QMessageBox>
#include <QDateTime>

#include "log.h"
#include "report.h"
#include "room.h"

class DAO : public QWidget
{
    Q_OBJECT
public:
    explicit DAO(QWidget *parent = nullptr);
    ~DAO();

    QList<Log> RequestLog(int roomId,QDateTime dateTime);// 根据房间号和时间段获取日志
    float RequestBillLog(int roomId,QDateTime dateTime);// 根据房间号和时间段获取日志，生成账单
    bool AddLog2DB(int eventId,Room room);// 把日志写入数据库，返回true成功，false失败
    bool AddLog2DB(Log log);// 把日志写入数据库，返回true成功，false失败
    QList<Report> RequestReportLog(QDateTime dateTime);// 根据请求的报表信息（房间Id列表，日期）获取相应日志，生成报告

signals:

private:
    QSqlDatabase db;// 数据库对象

    void InitDatabase();// 初始化
};

#endif // DAO_H
