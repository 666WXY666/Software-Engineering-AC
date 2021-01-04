#include "statistic.h"

Statistic::Statistic()
{

}

Statistic::Statistic(DAO *dao,QDateTime startTime)
{
    this->dao=dao;
    this->startTime=startTime;
}


// 打印账单相关
float Statistic::GetBill(int ID)
{
    return dao->RequestBillLog(ID,startTime);
}

bool Statistic::PrintBill(int ID)
{
    float bill=GetBill(ID);
    std::string id_Str = std::to_string(ID);
    std::string bill_Str = std::to_string(bill);
    std::ofstream outfile;
    outfile.open("bill.txt", std::ios::out | std::ios::app );
    if(outfile)
    {
        outfile<<"#################### "<<startTime.toString("yyyy-MM-dd hh:mm:ss").toStdString()<<" ####################\n";
        outfile<<"以下是"+id_Str+"号房间的账单："<<std::endl;
        outfile<<"-------------------------------------------------------------\n\n";
        outfile<<"\t房间："<<id_Str<<std::endl;
        outfile<<"\t总费用："<<bill_Str<<std::endl;
        outfile<<"\n-------------------------------------------------------------\n";
        outfile<<"#################### "<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString()<<" ####################\n\n";
        outfile.close();
        return true;
    }
    else
    {
        return false;
    }
}

void Statistic::GetSum()
{

}

// 打印详单相关
bool Statistic::PrintDetailList(int ID)
{
//    int id;// 日志ID，插入时不用设置，数据库自动生成
//    QDateTime timeStamp;// 日志产生时间，插入时不用设置，数据库自动生成
//    int eventId;// 位置事件编号
//    int roomId;// 房间号
//    int serverId;// 服务器号
//    int waitId;// 等待队列号，只有event_id=5时有效，默认为-1
//    int targetTemp;// 目标温度
//    int fanSpeed;// 风速
//    float currentTemp;// 当前温度
//    float totalFee;// 总费用
//    int serveTime;// 服务时间
//    float feeRate;// 费率
    QList<Log>loglist=GetDetailList(ID);
    std::string id_Str = std::to_string(ID);
    std::ofstream outfile;
    outfile.open("detail_list.txt", std::ios::out | std::ios::app);
    if(outfile)
    {
        outfile<<"########################################### "<<startTime.toString("yyyy-MM-dd hh:mm:ss").toStdString()<<" ###########################################\n";
        outfile<<"以下是"+id_Str+"号房间的详单："<<std::endl;
        outfile<<"-----------------------------------------------------------------------------------------------------------\n";
        outfile<<"\t时间\t\t\t\t事件\t\t当前温度\t目标温度\t风速\t服务ID\t\t服务时间\t费率\t\t总费用"<<std::endl;
        outfile<<"-----------------------------------------------------------------------------------------------------------\n";
        for(int i=0;i<loglist.size();i++){
            Log temp =loglist.at(i);
            std::string startTime =temp.getTimeStamp().toString().toStdString();// 时间
            std::string tem=std::to_string(temp.getCurrentTemp());// 当前温度
            int fan=temp.getFanSpeed();// 风速
            QString fanSpeedStr;
            switch (fan)
            {
            case SERVER_FAN_LOW:
                fanSpeedStr="低";
                break;
            case SERVER_FAN_MID:
                fanSpeedStr="中";
                break;
            case SERVER_FAN_HIGH:
                fanSpeedStr="高";
                break;
            default:
                fanSpeedStr="未知";
                break;
            }
            std::string targetTemp=std::to_string(temp.getTargetTemp());// 目标温度
            std::string serveId=std::to_string(temp.getServerId());
            std::string serveTime=std::to_string(temp.getServeTime());// 服务时间
            std::string feeRate=std::to_string(temp.getFeeRate());// 费率
            std::string totalFee=std::to_string(temp.getTotalFee());//总费用
            int event=temp.getEventId();
            QString eventStr;
            switch (event)
            {
            case EVENT_REQUEST_ON:
                eventStr="开机\t";
                break;
            case EVENT_REQUEST_OFF:
                eventStr="关机\t";
                break;
            case EVENT_SERVE_ON:
                eventStr="开启服务";
                break;
            case EVENT_SERVE_OFF:
                eventStr="关闭服务";
                break;
            case EVENT_CHANGE_TEMP:
                eventStr="修改温度";
                break;
            case EVENT_CHANGE_FAN_SPEED:
                eventStr="修改风速";
                break;
            default:
                eventStr="未知\t";
                break;
            }
            outfile<<startTime<<"\t"<<eventStr.toStdString()<<"\t"<<tem<<"\t"<<targetTemp<<"\t"<<"\t"<<fanSpeedStr.toStdString()<<"\t"<<serveId<<"\t"<<"\t"<<serveTime<<"\t"<<"\t"<<feeRate<<"\t"<<totalFee<<std::endl;
        }
        outfile<<"-----------------------------------------------------------------------------------------------------------\n";
        outfile<<"########################################### "<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString()<<" ###########################################\n\n";
        outfile.close();
        return true;
    }
    else
    {
        return false;
    }
}

void Statistic::ProcessLog()
{

}

void Statistic::AddDetail()
{

}

//打印报表相关
QList<Report> Statistic::GetReport()
{
    QList<Report> reportList = dao->RequestReportLog(startTime);
    return reportList;
}

bool Statistic::PrintReport()
{
    QList<Report> reportList = GetReport();
    std::ofstream outfile;
    outfile.open("report.txt", std::ios::out | std::ios::app);
    if(outfile)
    {
        outfile<<"#################### "<<startTime.toString("yyyy-MM-dd hh:mm:ss").toStdString()<<" ####################\n";
        outfile<<"以下是本次各房间空调工作数据："<<std::endl;
        outfile<<"-------------------------------------------------------------\n";
        for(int i=0;i<reportList.size();i++)
        {
            Report tempReport = reportList.at(i);
            std::string id_Str = std::to_string(tempReport.getRoomId());
            std::string timeOnOff_Str = std::to_string(tempReport.getTimesOfOnOff());
            std::string duration_Str = std::to_string(tempReport.getDuration());
            std::string totalFee_Str = std::to_string(tempReport.getTotalFee());
            std::string timesOfDispatch_Str = std::to_string(tempReport.getTimesOfDispatch());
            std::string numberOfRDR_Str = std::to_string(tempReport.getNumberOfRDR());
            std::string timesOfChangeTemp_Str = std::to_string(tempReport.getTimesOfChangeTemp());
            std::string timesOfChangeFanSpeed_Str = std::to_string(tempReport.getTimesOfChangeFanSpeed());
            outfile<<std::endl;
            outfile<<"\t房间号："<<id_Str<<std::endl;
            outfile<<"\t空调开关次数："<<timeOnOff_Str<<std::endl;
            outfile<<"\t空调使用时长："<<duration_Str<<std::endl;
            outfile<<"\t总费用："<<totalFee_Str<<std::endl;
            outfile<<"\t服务器调度次数："<<timesOfDispatch_Str<<std::endl;
            outfile<<"\t详单数："<<numberOfRDR_Str<<std::endl;
            outfile<<"\t调温次数："<<timesOfChangeTemp_Str<<std::endl;
            outfile<<"\t调风次数："<<timesOfChangeFanSpeed_Str<<std::endl;
            outfile<<"\n-------------------------------------------------------------\n";
        }
        outfile<<"#################### "<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString()<<" ####################\n\n";
        outfile.close();
        return true;
    }
    else
    {
        return false;
    }
}
