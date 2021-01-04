#include "dao.h"

DAO::DAO(QWidget *parent)
    : QWidget(parent)
{
    InitDatabase();
}

DAO::~DAO()
{
    // 关闭sys数据库
    db.close();
}

void DAO::InitDatabase()// 初始化
{
    qDebug()<<"支持的驱动："<<QSqlDatabase::drivers();
    // 判断sys是否已经连接
    if(QSqlDatabase::contains("qt_sql_default_connection"))
        db = QSqlDatabase::database("qt_sql_default_connection");
    else
        db = QSqlDatabase::addDatabase("QMYSQL");
    // 设置连接参数
    db.setHostName("127.0.0.1");
    db.setUserName("wxy");
    db.setPassword("123456");
    db.setDatabaseName("sys");
    // 数据库打开失败
    if(!db.open())
    {
        qDebug()<<"数据库sys打开失败"<<db.lastError().text();
        QMessageBox::warning(this,"数据库info打开失败",db.lastError().text());
        return;
    }
    // 如果不存在，创建数据库
    QSqlQuery queryCreateDB;
    if(!queryCreateDB.exec("CREATE DATABASE IF NOT EXISTS ac_log DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;"))
    {
        qDebug()<<"数据库ac_log创建失败"<<queryCreateDB.lastError().text();
        QMessageBox::warning(this,"数据库ac_log创建失败",queryCreateDB.lastError().text());
        return;
    }
    else
    {
        qDebug()<<"数据库ac_log创建成功";
    }
    // 关闭sys数据库
    db.close();
    // 打开新创建的ac_log数据库
    db.setDatabaseName("ac_log");
    if(!db.open())
    {
        qDebug()<<"数据库ac_log打开失败"<<db.lastError().text();
        QMessageBox::warning(this,"数据库ac_log打开失败",db.lastError().text());
        return;
    }
    else
    {
        qDebug()<<"数据库ac_log打开成功";
    }
    QSqlQuery queryCreateTableLog;
    if(!queryCreateTableLog.exec("CREATE TABLE IF NOT EXISTS log("
                                "id INT PRIMARY KEY NOT NULL AUTO_INCREMENT,"
                                "time_stamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
                                "event_id INT DEFAULT -1 CHECK(event_id BETWEEN -1 AND 6),"
                                "room_id INT DEFAULT -1 CHECK(room_id >= -1),"
                                "server_id INT DEFAULT -1 CHECK(server_id BETWEEN -1 AND 2),"
                                "wait_id INT DEFAULT -1 CHECK(wait_id >= -1),"
                                "target_temp INT DEFAULT -1,"
                                "fan_speed INT DEFAULT 1 CHECK(fan_speed BETWEEN 0 AND 2),"
                                "current_temp FLOAT DEFAULT -1,"
                                "total_fee FLOAT DEFAULT 0 CHECK(total_fee >= 0),"
                                "serve_time INT DEFAULT 0 CHECK(serve_time >= 0),"
                                "fee_rate FLOAT DEFAULT 0 CHECK(fee_rate >= 0));"))
    {
        qDebug()<<"数据库表log创建失败"<<queryCreateTableLog.lastError().text();
        QMessageBox::warning(this,"数据库表log创建失败",queryCreateTableLog.lastError().text());
        return;
    }
    else
    {
        qDebug()<<"数据库表log创建成功";
    }
}

QList<Log> DAO::RequestLog(int roomId,QDateTime dateTime)// 根据房间号和时间获取日志
{
    QSqlQuery queryRequestLog;
    QString sqlRequestLog=QString("SELECT * FROM log WHERE room_id = %1 AND time_stamp > '%2';").arg(roomId).arg(dateTime.toString("yyyy-MM-dd hh:mm:ss"));
    if(!queryRequestLog.exec(sqlRequestLog))
    {
        qDebug()<<"获取详单数据-查询日志数据库失败"<<queryRequestLog.lastError().text();
        QMessageBox::warning(this,"获取详单数据-查询日志数据库失败",queryRequestLog.lastError().text());
        QList<Log> errorLogList;
        return errorLogList;
    }
    else
    {
        qDebug()<<"获取详单数据-查询日志数据库成功";
        QList<Log> logList;
        while (queryRequestLog.next())
        {
            Log log(queryRequestLog.value("id").toInt(),queryRequestLog.value("time_stamp").toDateTime(),
                    queryRequestLog.value("event_id").toInt(),queryRequestLog.value("room_id").toInt(),
                    queryRequestLog.value("server_id").toInt(),queryRequestLog.value("wait_id").toInt(),
                    queryRequestLog.value("target_temp").toInt(),queryRequestLog.value("fan_speed").toInt(),
                    queryRequestLog.value("current_temp").toFloat(),queryRequestLog.value("total_fee").toFloat(),
                    queryRequestLog.value("serve_time").toInt(),queryRequestLog.value("fee_rate").toFloat());
            logList.append(log);
        }
        return logList;
    }
}

float DAO::RequestBillLog(int roomId,QDateTime dateTime)// 根据房间号和时间段获取日志，生成账单
{
    QSqlQuery queryRequestBillLog;
    QString sqlRequestBillLog=QString("SELECT SUM(total_fee) FROM log WHERE room_id=%1 AND event_id=4 AND time_stamp > '%2';").arg(roomId).arg(dateTime.toString("yyyy-MM-dd hh:mm:ss"));
    if(!queryRequestBillLog.exec(sqlRequestBillLog))
    {
        qDebug()<<"获取账单数据-查询日志数据库失败"<<queryRequestBillLog.lastError().text();
        QMessageBox::warning(this,"获取账单数据-查询日志数据库失败",queryRequestBillLog.lastError().text());
        return -1;
    }
    else
    {
        qDebug()<<"获取账单数据-查询日志数据库成功";
        queryRequestBillLog.next();
        return queryRequestBillLog.value(0).toFloat();
    }
}

bool DAO::AddLog2DB(int eventId,Room room)// 把日志写入数据库，返回true成功，false失败
{
    QSqlQuery queryAddLog2DB;
    QString sqlAddLog2DB=QString("INSERT INTO log(event_id, room_id, server_id, wait_id, target_temp, fan_speed, current_temp, total_fee, serve_time, fee_rate) VALUES (%1, %2, %3, %4, %5, %6, %7, %8, %9, %10);")
                                 .arg(eventId)
                                 .arg(room.roomId)
                                 .arg(room.serverId)
                                 .arg(room.waitId)
                                 .arg(room.targetTemp)
                                 .arg(room.currentFanSpeed)
                                 .arg(room.currentTemp)
                                 .arg(room.fee)
                                 .arg(int(room.serverTime))
                                 .arg(room.feeRate);
    if(!queryAddLog2DB.exec(sqlAddLog2DB))
    {
        qDebug()<<"把日志写入数据库失败"<<queryAddLog2DB.lastError().text();
        QMessageBox::warning(this,"把日志写入数据库失败",queryAddLog2DB.lastError().text());
        return false;
    }
    else
    {
        return true;
    }
}

bool DAO::AddLog2DB(Log log)// 把日志写入数据库，返回true成功，false失败
{
    QSqlQuery queryAddLog2DB;
    QString sqlAddLog2DB=QString("INSERT INTO log(event_id, room_id, server_id, wait_id, target_temp, fan_speed, current_temp, total_fee, serve_time, fee_rate) VALUES (%1, %2, %3, %4, %5, %6, %7, %8, %9, %10);")
                                 .arg(log.getEventId())
                                 .arg(log.getRoomId())
                                 .arg(log.getServerId())
                                 .arg(log.getWaitId())
                                 .arg(log.getTargetTemp())
                                 .arg(log.getFanSpeed())
                                 .arg(log.getCurrentTemp())
                                 .arg(log.getTotalFee())
                                 .arg(log.getServeTime())
                                 .arg(log.getFeeRate());
    if(!queryAddLog2DB.exec(sqlAddLog2DB))
    {
        qDebug()<<"把日志写入数据库失败"<<queryAddLog2DB.lastError().text();
        QMessageBox::warning(this,"把日志写入数据库失败",queryAddLog2DB.lastError().text());
        return false;
    }
    else
    {
        return true;
    }
}

QList<Report> DAO::RequestReportLog(QDateTime dateTime)// 根据请求的报表信息（房间Id列表，日期）获取相应日志，生成报告
{
    QSqlQuery queryRequestReportLog;
    QString sqlRoomId=QString("SELECT DISTINCT(room_id) FROM log WHERE time_stamp > '%1' ORDER BY room_id;").arg(dateTime.toString("yyyy-MM-dd hh:mm:ss"));
    if(!queryRequestReportLog.exec(sqlRoomId))
    {
        qDebug()<<"获取报告房间ID-查询日志数据库失败"<<queryRequestReportLog.lastError().text();
        QMessageBox::warning(this,"获取报告房间ID-查询日志数据库失败",queryRequestReportLog.lastError().text());
        QList<Report> errorReportList;
        return errorReportList;
    }
    else
    {
        QList<Report> reportList;
        qDebug()<<"获取报告房间ID-查询日志数据库成功";
        while (queryRequestReportLog.next())
        {
            int roomId=queryRequestReportLog.value(0).toInt();
            QSqlQuery queryTimesOfOnOff,queryDuration,queryTotalFee,queryTimesOfDispatch,
                      queryNumberOfRDR,queryTimesOfChangeTemp,queryTimesOfChangeFanSpeed;
            QString sqlTimesOfOnOff=QString("SELECT COUNT(*) FROM log WHERE room_id=%1 AND (event_id=0 OR event_id=4) AND time_stamp > '%2';").arg(roomId).arg(dateTime.toString("yyyy-MM-dd hh:mm:ss"));
            QString sqlDuration=QString("SELECT SUM(serve_time) FROM log WHERE room_id=%1 AND event_id=5 AND time_stamp > '%2';").arg(roomId).arg(dateTime.toString("yyyy-MM-dd hh:mm:ss"));
            QString sqlTotalFee=QString("SELECT SUM(total_fee) FROM log WHERE room_id=%1 AND event_id=4 AND time_stamp > '%2';").arg(roomId).arg(dateTime.toString("yyyy-MM-dd hh:mm:ss"));
            QString sqlTimesOfDispatch=QString("SELECT COUNT(*) FROM log WHERE room_id=%1 AND event_id=1 AND time_stamp > '%2';").arg(roomId).arg(dateTime.toString("yyyy-MM-dd hh:mm:ss"));
            QString sqlNumberOfRDR=QString("SELECT COUNT(*) FROM log WHERE room_id=%1 AND time_stamp > '%2';").arg(roomId).arg(dateTime.toString("yyyy-MM-dd hh:mm:ss"));
            QString sqlTimesOfChangeTemp=QString("SELECT COUNT(*) FROM log WHERE room_id=%1 AND event_id=2 AND time_stamp > '%2';").arg(roomId).arg(dateTime.toString("yyyy-MM-dd hh:mm:ss"));
            QString sqlTimesOfChangeFanSpeed=QString("SELECT COUNT(*) FROM log WHERE room_id=%1 AND event_id=3 AND time_stamp > '%2';").arg(roomId).arg(dateTime.toString("yyyy-MM-dd hh:mm:ss"));
            if(queryTimesOfOnOff.exec(sqlTimesOfOnOff)&&queryDuration.exec(sqlDuration)&&queryTotalFee.exec(sqlTotalFee)&&queryTimesOfDispatch.exec(sqlTimesOfDispatch)&&
               queryNumberOfRDR.exec(sqlNumberOfRDR)&&queryTimesOfChangeTemp.exec(sqlTimesOfChangeTemp)&&queryTimesOfChangeFanSpeed.exec(sqlTimesOfChangeFanSpeed))
            {
                qDebug()<<"获取报告数据-查询日志数据库成功";
                queryTimesOfOnOff.next();
                queryDuration.next();
                queryTotalFee.next();
                queryTimesOfDispatch.next();
                queryNumberOfRDR.next();
                queryTimesOfChangeTemp.next();
                queryTimesOfChangeFanSpeed.next();
                Report report(roomId,
                              queryTimesOfOnOff.value(0).toInt(),
                              queryDuration.value(0).toInt(),
                              queryTotalFee.value(0).toFloat(),
                              queryTimesOfDispatch.value(0).toInt(),
                              queryNumberOfRDR.value(0).toInt(),
                              queryTimesOfChangeTemp.value(0).toInt(),
                              queryTimesOfChangeFanSpeed.value(0).toInt());
                reportList.append(report);
            }
            else
            {
                qDebug()<<"获取报告数据-查询日志数据库失败";
                QMessageBox::warning(this,"获取报告数据","查询日志数据库失败");
                QList<Report> errorReportList;
                return errorReportList;
            }
        }
        return reportList;
    }
}

