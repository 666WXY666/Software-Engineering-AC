#ifndef STATISTIC_H
#define STATISTIC_H

#include "dao.h"
#include "log.h"
#include "server.h"
#include <fstream>
#include <iostream>
#include <string>
#include <QList>

class Statistic
{
public:
    Statistic();
    Statistic(DAO *dao,QDateTime startTime);

    QDateTime startTime;
    DAO *dao;

    // 打印账单相关
    float GetBill(int ID);
    void GetSum();
    bool PrintBill(int ID);

    // 打印详单相关
    void ProcessLog();
    void AddDetail();

    QList<Log> GetDetailList(int ID)
    {
        QList<Log> loglist = dao->RequestLog(ID,startTime);
        return loglist;
    }
    bool PrintDetailList(int ID);


    // 打印报表相关
    QList<Report> GetReport();
    bool PrintReport();
    void GetView();

private:
    QDateTime requestTime;
    int requestDuration;
    int fanSpeed;
    float feeRate;
    float fee;
    QDateTime dateIn;
    QDateTime dateOut;
    float totalFee;
    QList<int> roomList;
    int reportType;
    QDateTime dateFrom;
    QDateTime dateTo;
    int reportId;
    //timer();
};

#endif // STATISTIC_H
