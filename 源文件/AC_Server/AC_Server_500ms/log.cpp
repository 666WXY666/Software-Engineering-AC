#include "log.h"

Log::Log():id(0),timeStamp(QDateTime::currentDateTime()),eventId(-1),roomId(-1),serverId(-1),
           waitId(-1),targetTemp(-1),fanSpeed(1),currentTemp(-1),totalFee(0),serveTime(0),feeRate(0)
{

}

Log::Log(int id,QDateTime timeStamp,int eventId,int roomId,int serverId,int waitId,
         int targetTemp,int fanSpeed,float currentTemp,float totalFee,int serveTime,float feeRate)
{
    this->id=id;// 日志ID，插入时不用设置，数据库自动生成
    this->timeStamp=timeStamp;// 日志产生时间，插入时不用设置，数据库自动生成
    this->eventId=eventId;// 位置事件编号
    this->roomId=roomId;// 房间号
    this->serverId=serverId;// 服务器号
    this->waitId=waitId;// 等待队列号，只有event_id=5时有效，默认为-1
    this->targetTemp=targetTemp;// 目标温度
    this->fanSpeed=fanSpeed;// 风速
    this->currentTemp=currentTemp;// 当前温度
    this->totalFee=totalFee;// 总费用
    this->serveTime=serveTime;// 服务时间
    this->feeRate=feeRate;// 费率
}
