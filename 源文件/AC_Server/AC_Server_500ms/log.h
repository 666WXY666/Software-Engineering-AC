#ifndef LOG_H
#define LOG_H

#include <QDebug>
#include <QDateTime>

#define EVENT_REQUEST_ON 0// 开机
#define EVENT_SERVE_ON 1// 服务每开启一次算一次调度
#define EVENT_CHANGE_TEMP 2
#define EVENT_CHANGE_FAN_SPEED 3
#define EVENT_REQUEST_OFF 4// 关机
#define EVENT_SERVE_OFF 5

class Log
{
public:
    explicit Log();
    explicit Log(int id,QDateTime timeStamp,int eventId,int roomId,int serverId,int waitId,
                 int targetTemp,int fanSpeed,float currentTemp,float totalFee,int serveTime,float feeRate);

    int getId() {
        return id;
    }

    void setId(int id) {
        if(id>=0)
        {
            this->id = id;
        }
        else
        {
            qDebug()<<"日志ID设置超出范围";
        }
    }

    QDateTime getTimeStamp() {
        return timeStamp;
    }

    void setTimeStamp(QDateTime timeStamp) {
        this->timeStamp = timeStamp;
    }

    int getEventId() {
        return eventId;
    }

    void setEventId(int eventId) {
        if(eventId>=0&&eventId<=6)
        {
            this->eventId = eventId;
        }
        else
        {
            qDebug()<<"日志事件ID设置超出范围";
        }
    }

    int getRoomId() {
        return roomId;
    }

    void setRoomId(int roomId) {
        if(roomId>=0)
        {
            this->roomId = roomId;
        }
        else
        {
            qDebug()<<"房间ID设置超出范围";
        }
    }

    int getServerId() {
        return serverId;
    }

    void setServerId(int serverId) {
        if(serverId>=0&&serverId<=2)
        {
            this->serverId = serverId;
        }
        else
        {
            qDebug()<<"服务对象ID设置超出范围";
        }
    }

    int getWaitId() {
        return waitId;
    }

    void setWaitId(int waitId) {
        if(waitId>=0)
        {
            this->waitId = waitId;
        }
        else
        {
            qDebug()<<"等待队列ID设置超出范围";
        }
    }

    int getTargetTemp() {
        return targetTemp;
    }

    void setTargetTemp(int targetTemp) {
        this->targetTemp = targetTemp;
    }

    int getFanSpeed() {
        return fanSpeed;
    }

    void setFanSpeed(int fanSpeed) {
        if(fanSpeed>=0&&fanSpeed<=2)
        {
            this->fanSpeed = fanSpeed;
        }
        else
        {
            qDebug()<<"风速设置超出范围";
        }
    }

    float getCurrentTemp() {
        return currentTemp;
    }

    void setCurrentTemp(float currentTemp) {
        this->currentTemp = currentTemp;
    }

    float getTotalFee() {
        return totalFee;
    }

    void setTotalFee(float totalFee) {
        if(totalFee>=0)
        {
            this->totalFee = totalFee;
        }
        else
        {
            qDebug()<<"总金额设置超出范围";
        }
    }

    int getServeTime() {
        return serveTime;
    }

    void setServeTime(int serveTime) {
        if(serveTime>=0)
        {
            this->serveTime = serveTime;
        }
        else
        {
            qDebug()<<"服务时长设置超出范围";
        }
    }

    float getFeeRate() {
        return feeRate;
    }

    void setFeeRate(float feeRate) {
        if(feeRate>0)
        {
            this->feeRate = feeRate;
        }
        else
        {
            qDebug()<<"费率设置超出范围";
        }
    }

signals:

private:
    int id;// 日志ID，插入时不用设置，数据库自动生成
    QDateTime timeStamp;// 日志产生时间，插入时不用设置，数据库自动生成
    int eventId;// 位置事件编号
    int roomId;// 房间号
    int serverId;// 服务器号
    int waitId;// 等待队列号，只有event_id=5时有效，默认为-1
    int targetTemp;// 目标温度
    int fanSpeed;// 风速
    float currentTemp;// 当前温度
    float totalFee;// 总费用
    int serveTime;// 服务时间
    float feeRate;// 费率
};

#endif // LOG_H
