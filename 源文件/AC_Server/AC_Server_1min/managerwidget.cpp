#include "managerwidget.h"
#include "ui_managerwidget.h"

ManagerWidget::ManagerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ManagerWidget)
{
    ui->setupUi(this);

    connect(ui->BacktoWidgetButton,&QPushButton::clicked,this,&ManagerWidget::ManagertoWidget);// 按下返回按钮返回主界面
    connect(ui->ShowButton,&QPushButton::clicked,this,&ManagerWidget::PrintReports);// 打印报表
    connect(ui->pushButton_3,&QPushButton::clicked,this,&ManagerWidget::ShowGraph);// 显示图形
}

ManagerWidget::~ManagerWidget()
{
    delete ui;
}

////////////////////////////////////////////////////////////// 以下为槽函数 ////////////////////////////////////////////////////////////////////

void ManagerWidget::ManagertoWidget()// 槽函数，用来发送返回主界面的信号
{
    emit ManagerBacktoWidget();
}

void ManagerWidget::PrintReports()
{
    if(statistic->PrintReport())
    {
        QMessageBox::information(this,"通知","生成报表成功");
        qDebug()<<"生成账报表成功";
    }
    else
    {
        QMessageBox::warning(this,"警告","生成报表失败");
        qDebug()<<"生成报表失败";
    }
}

void ManagerWidget::ShowGraph()
{
    QList<Report> reports=statistic->GetReport();

    QChart* m_chart = new QChart();// 绘制饼图
    QPieSeries* m_series = new QPieSeries();
    for(int i=0;i<reports.size();i++)
    {
        Report tempReport=reports.at(i);
        m_series->append(QString::number(tempReport.getRoomId()),tempReport.getTotalFee());
    }
    m_chart->setTitle("房间总费用占比");
    m_chart->addSeries(m_series);
    ui->PieWidget->setChart(m_chart);


    QChart* chart= new QChart();// 绘制条形图
    QBarSet* feeSet= new QBarSet("总费用");
    for(int i=0;i<reports.size();i++)
    {
        Report tempReport=reports.at(i);
        *feeSet<<tempReport.getTotalFee();
    }
    feeSet->setLabelColor(QColor(0,0,255));

    QBarSeries *series = new QBarSeries();
    series->append(feeSet);
    series->setVisible(true);
    series->setLabelsVisible(true);

    chart->setTheme(QChart::ChartThemeLight);//设置白色主题
    chart->setDropShadowEnabled(true);//背景阴影
    chart->addSeries(series);//添加系列到QChart上

    chart->setTitleBrush(QBrush(QColor(0,0,255)));//设置标题Brush
    chart->setTitleFont(QFont("微软雅黑"));//设置标题字体
    chart->setTitle("房间总费用统计");

     QBarCategoryAxis *axisX = new QBarCategoryAxis;
     for(int i=0;i<reports.size();i++)
     {
         Report tempReport=reports.at(i);
         axisX->append(QString::number(tempReport.getRoomId()));
     }
     QValueAxis *axisY = new QValueAxis;
     axisY->setRange(0,20);
     axisY->setTitleText("费用(单位:元)");
     axisY->setLabelFormat("%d");

     chart->setAxisX(axisX,series);
     chart->setAxisY(axisY,series);

     chart->legend()->setVisible(true);
     chart->legend()->setAlignment(Qt::AlignBottom);//底部对齐
     chart->legend()->setBackgroundVisible(true);//设置背景是否可视
     chart->legend()->setAutoFillBackground(true);//设置背景自动填充
     chart->legend()->setColor(QColor(222,233,251));//设置颜色
     chart->legend()->setLabelColor(QColor(0,100,255));//设置标签颜色
     //chart->legend()->setMaximumHeight(50);
     ui->LineWidget->setChart(chart);
}
