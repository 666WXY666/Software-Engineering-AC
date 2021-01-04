#ifndef REPORT_H
#define REPORT_H

#include <QDebug>

class Report
{
public:
    explicit Report();
    explicit Report(int roomId,int timesOfOnOff,int duration,float totalFee,int timesOfDispatch,
                    int numberOfRDR,int timesOfChangeTemp,int timesOfChangeFanSpeed);

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
            qDebug()<<"报告ID设置超出范围";
        }
    }

    int getTimesOfOnOff() {
        return timesOfOnOff;
    }

    void setTimesOfOnOff(int timesOfOnOff) {
        if(timesOfOnOff>=0)
        {
            this->timesOfOnOff = timesOfOnOff;
        }
        else
        {
            qDebug()<<"报告开关次数设置超出范围";
        }
    }

    int getDuration() {
        return duration;
    }

    void setDuration(int duration) {
        if(duration>=0)
        {
            this->duration = duration;
        }
        else
        {
            qDebug()<<"报告时长设置超出范围";
        }
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
            qDebug()<<"报告总费用设置超出范围";
        }
    }

    int getTimesOfDispatch() {
        return timesOfDispatch;
    }

    void setTimesOfDispatch(int timesOfDispatch) {
        if(timesOfDispatch>=0)
        {
            this->timesOfDispatch = timesOfDispatch;
        }
        else
        {
            qDebug()<<"报告调度次数设置超出范围";
        }
    }

    int getNumberOfRDR() {
        return numberOfRDR;
    }

    void setNumberOfRDR(int numberOfRDR) {
        if(numberOfRDR>=0)
        {
            this->roomId = numberOfRDR;
        }
        else
        {
            qDebug()<<"报告详单次数设置超出范围";
        }
    }

    int getTimesOfChangeTemp() {
        return timesOfChangeTemp;
    }

    void setTimesOfChangeTemp(int timesOfChangeTemp) {
        if(timesOfChangeTemp>=0)
        {
            this->timesOfChangeTemp = timesOfChangeTemp;
        }
        else
        {
            qDebug()<<"报告调温次数设置超出范围";
        }
    }

    int getTimesOfChangeFanSpeed() {
        return timesOfChangeFanSpeed;
    }

    void setTimesOfChangeFanSpeed(int timesOfChangeFanSpeed) {
        if(timesOfChangeFanSpeed>=0)
        {
            this->timesOfChangeFanSpeed = timesOfChangeFanSpeed;
        }
        else
        {
            qDebug()<<"报告调风次数设置超出范围";
        }
    }

signals:

private:
    int roomId;// 房间ID
    int timesOfOnOff;// 开关次数
    int duration;// 使用空调的时长，单位s
    float totalFee;// 总费用
    int timesOfDispatch;// 被调度的次数
    int numberOfRDR;// 详单数
    int timesOfChangeTemp;// 调温次数
    int timesOfChangeFanSpeed;// 调风次数
};

#endif // REPORT_H
