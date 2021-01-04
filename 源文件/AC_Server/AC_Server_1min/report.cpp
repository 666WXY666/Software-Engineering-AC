#include "report.h"

Report::Report() : roomId(-1),timesOfOnOff(0),duration(0),totalFee(0),timesOfDispatch(0),
                   numberOfRDR(0),timesOfChangeTemp(0),timesOfChangeFanSpeed(0)
{

}

Report::Report(int roomId,int timesOfOnOff,int duration,float totalFee,int timesOfDispatch,
               int numberOfRDR,int timesOfChangeTemp,int timesOfChangeFanSpeed)
{
    this->roomId=roomId;
    this->timesOfOnOff=timesOfOnOff;
    this->duration=duration;
    this->totalFee=totalFee;
    this->timesOfDispatch=timesOfDispatch;
    this->numberOfRDR=numberOfRDR;
    this->timesOfChangeTemp=timesOfChangeTemp;
    this->timesOfChangeFanSpeed=timesOfChangeFanSpeed;
}
