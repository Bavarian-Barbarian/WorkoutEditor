/*
 * Copyright (c) 2016 Andreas Hunner (andy-atech@gmx.net)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "week_popup.h"
#include "ui_week_popup.h"

week_popup::week_popup(QWidget *parent,QString weekinfo,schedule *p_sched) :
    QDialog(parent),
    ui(new Ui::week_popup)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    isLoad = false;
    dayCount = 7;
    workSched = p_sched;
    week_info << weekinfo.split("#");
    workProxy = new QSortFilterProxyModel;
    workProxy->setSourceModel(workSched->workout_schedule);
    barSelection << "Duration" << "Distance";
    ui->comboBox_yValue->addItems(barSelection);
    xStress.resize(dayCount);
    xLTS.resize(dayCount+1);
    yLTS.resize(dayCount+1);
    yStress.resize(dayCount);
    xBar.resize(dayCount);
    yDura.resize(dayCount);
    yDist.resize(dayCount);
    xWorks.resize(dayCount);
    yWorks.resize(dayCount);
    yWorkCount.resize(dayCount);
    yValues.resize(dayCount);
    maxValues.resize(3);
    maxValues.fill(0);
    this->set_plotValues();
}

enum {DURATION,DISTANCE};

week_popup::~week_popup()
{
    delete ui;
    delete workProxy;
}

void week_popup::set_plotValues()
{
    workProxy->setFilterRegExp("\\b"+week_info.at(0)+"\\b");
    workProxy->setFilterKeyColumn(0);
    workProxy->sort(0);
    int proxyCount = workProxy->rowCount();
    QDate tempDate;

    if(proxyCount > 0)
    {
        QDateTime weekStart,workoutDate;
        QTime wTime;
        wTime.fromString("00:00:00","hh:mm:ss");
        tempDate = QDate::fromString(workProxy->data(workProxy->index(0,1)).toString(),"dd.MM.yyyy");
        weekStart.setDate(tempDate.addDays(1 - tempDate.dayOfWeek()));
        weekStart.setTime(wTime);
        weekStart.setTimeSpec(Qt::UTC);
        firstDay = weekStart.date();

        for(int i = 0; i < dayCount; ++i)
        {
            weekDates.insert(i,weekStart.addDays(i));
        }

        double stress,dura,dist;

        ui->label_weekinfos->setText("Week: " + week_info.at(0) + " - Phase: " + week_info.at(1) + " - Workouts: " + QString::number(proxyCount));

        double dateValue = 0;
        double ltsDays = settings::get_ltsValue("ltsdays");
        double lte = (double)exp(-1.0/ltsDays);
        int ltsStart = -ltsDays;
        double ltsStress = 0,currStress = 0,pastStress = 0,startLTS = 0;
        QMap<QDate,double> *stressMap = workSched->get_StressMap();
        pastStress = settings::get_ltsValue("lastlts");

        for(QMap<QDate,double>::const_iterator it = stressMap->cbegin(), end = stressMap->find(weekDates.at(0).date().addDays(ltsStart)); it != end; ++it)
        {
            currStress = it.value();
            ltsStress = (currStress * (1.0 - lte)) + (pastStress * lte);
            pastStress = ltsStress;
        }
        startLTS = pastStress;
        xLTS[0] = weekDates.at(0).addDays(-1).toTime_t();

        for(int i = 0; i < dayCount; ++i)
        {
            pastStress = startLTS;
            dateValue = weekDates.at(i).toTime_t();
            xStress[i] = dateValue;
            xBar[i] = dateValue;
            xWorks[i] = dateValue;
            xLTS[i+1] = dateValue;

            for(int x = ltsStart; x <= 0; ++x)
            {
                if(i == 0 && x == 0) yLTS[0] = round(pastStress);
                currStress = stressMap->value(weekDates.at(i).date().addDays(x));
                ltsStress = (currStress * (1.0 - lte)) + (pastStress * lte);
                pastStress = ltsStress;
                if(x == ltsStart) startLTS = ltsStress;

            }
            yLTS[i+1] = round(ltsStress);
        }

        for(int i = 0,day = 0; i < proxyCount; ++i)
        {
            workoutDate = QDateTime::fromString(workProxy->data(workProxy->index(i,1,QModelIndex())).toString(),"dd.MM.yyyy");
            workoutDate.setTime(wTime);
            workoutDate.setTimeSpec(Qt::UTC);

            stress = workProxy->data(workProxy->index(i,8,QModelIndex())).toDouble();
            dura = static_cast<double>(this->get_timesec(workProxy->data(workProxy->index(i,6,QModelIndex())).toString())) / 60;
            dist = workProxy->data(workProxy->index(i,7,QModelIndex())).toDouble();

            for( ; day < weekDates.count(); ++day)
            {
                if(workoutDate == weekDates.at(day))
                {
                    yStress[day] = yStress[day] + stress;
                    yDura[day] = yDura[day] + this->set_doubleValue(dura,false);
                    yDist[day] = yDist[day] + dist;
                    yWorkCount[day] = yWorkCount[day]+1;
                }
                if(maxValues[0] < yStress[day]) maxValues[0] = yStress[day];
                if(maxValues[1] < yDura[day]) maxValues[1] = yDura[day];
                if(maxValues[2] < yDist[day]) maxValues[2] = yDist[day];
            }
            day = 0;
        }

        for(int i = 0; i < yWorkCount.count(); ++i)
        {
            yWorks[i] = yWorkCount[i]*10;
        }

        this->set_graph();
        this->set_weekPlot(0);
    }
    else
    {
        ui->label_weekinfos->setText("Week: " + week_info.at(0) + " - Phase: " + week_info.at(1) + " - Workouts: " + QString::number(proxyCount));
        this->set_graph();
    }
}

void week_popup::set_graph()
{
    QFont plotFont;
    plotFont.setBold(true);
    plotFont.setPointSize(8);

    ui->widget_plot->xAxis->setLabel("Day of Week");
    ui->widget_plot->xAxis->setLabelFont(plotFont);
    ui->widget_plot->xAxis2->setVisible(true);
    ui->widget_plot->xAxis2->setLabelFont(plotFont);
    ui->widget_plot->xAxis2->setTickLabels(false);
    ui->widget_plot->yAxis->setLabel("Stress");
    ui->widget_plot->yAxis->setLabelFont(plotFont);
    ui->widget_plot->yAxis2->setVisible(true);
    ui->widget_plot->yAxis2->setLabelFont(plotFont);
    ui->widget_plot->legend->setVisible(true);
    ui->widget_plot->legend->setFont(plotFont);

    QCPLayoutGrid *subLayout = new QCPLayoutGrid;
    ui->widget_plot->plotLayout()->addElement(1,0,subLayout);
    subLayout->setMargins(QMargins(dayCount*10,0,dayCount*10,5));
    subLayout->addElement(0,0,ui->widget_plot->legend);


    ui->widget_plot->addLayer("abovemain", ui->widget_plot->layer("main"), QCustomPlot::limAbove);
    ui->widget_plot->addLayer("belowmain", ui->widget_plot->layer("main"), QCustomPlot::limBelow);
}

void week_popup::set_weekPlot(int yValue)
{
    ui->widget_plot->clearPlottables();
    ui->widget_plot->clearItems();
    ui->widget_plot->legend->setFillOrder(QCPLegend::foColumnsFirst);
    ui->widget_plot->plotLayout()->setRowStretchFactor(1,0.0001);

    QCPRange xRange(QCPAxisTickerDateTime::dateTimeToKey(firstDay.addDays(-1)),QCPAxisTickerDateTime::dateTimeToKey(firstDay.addDays(dayCount)));
    QFont lineFont,barFont;
    lineFont.setPointSize(10);
    barFont.setPointSize(10);

    //QCPGraph *stressLine = this->get_QCPLine(ui->widget_plot,"StessScore",,,false);

    QCPGraph *stressLine = ui->widget_plot->addGraph();
    stressLine->setName("StressScore");
    stressLine->setLineStyle(QCPGraph::lsLine);
    stressLine->setData(xStress,yStress);
    stressLine->setAntialiased(true);
    stressLine->setBrush(QBrush(QColor(255,0,0,50)));
    stressLine->setPen(QPen(QColor(255,0,0),2));

    QCPGraph *ltsLine = ui->widget_plot->addGraph();
    ltsLine->setName("LTS");
    ltsLine->setLineStyle(QCPGraph::lsLine);
    ltsLine->setData(xLTS,yLTS);
    ltsLine->setAntialiased(true);
    //ltsLine->setBrush(QBrush(QColor(0,255,0,50)));
    ltsLine->setPen(QPen(QColor(0,255,0),2));

    QCPBars *workbars = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis);
    workbars->setName("Workouts");
    workbars->setWidth(dayCount*3000);
    workbars->setData(xWorks,yWorks);
    workbars->setAntialiased(true);
    workbars->setBrush(QBrush(QColor(255,255,100,80)));
    workbars->setPen(QPen(QColor(225,225,100)));

    QCPBars *bars = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis2);
    bars->setWidth(dayCount*6000);
    bars->setAntialiased(true);
    bars->setBrush(QBrush(QColor(0,85,255,70)));
    bars->setPen(QPen(Qt::darkBlue));

    stressLine->setLayer("abovemain");
    workbars->setLayer("abovemain");
    bars->setLayer("belowmain");
    ui->widget_plot->xAxis->grid()->setLayer("belowmain");
    ui->widget_plot->yAxis->grid()->setLayer("belowmain");

    ui->widget_plot->yAxis->setRange(0,maxValues[0]+20);

    if(yValue == DURATION)
    {
        qCopy(yDura.begin(),yDura.end(),yValues.begin());
        bars->setName(barSelection.at(0));
        bars->setData(xBar,yValues);
        ui->widget_plot->yAxis2->setRange(0,maxValues[1]+0.5);
        ui->widget_plot->yAxis2->setLabel(barSelection.at(0));
    }
    if(yValue == DISTANCE)
    {
        qCopy(yDist.begin(),yDist.end(),yValues.begin());
        bars->setName(barSelection.at(1));
        bars->setData(xBar,yValues);
        ui->widget_plot->yAxis2->setRange(0,maxValues[2]+5);
        ui->widget_plot->yAxis2->setLabel(barSelection.at(1));
    }

    for(int i = 0; i < dayCount; ++i)
    {
        QCPItemTracer *itemTracer = new QCPItemTracer(ui->widget_plot);
        itemTracer->setGraph(stressLine);
        itemTracer->setGraphKey(xStress[i]);
        itemTracer->setStyle(QCPItemTracer::tsCircle);
        itemTracer->setBrush(Qt::red);

        QCPItemText *lineText = new QCPItemText(ui->widget_plot);
        lineText->position->setType(QCPItemPosition::ptPlotCoords);
        lineText->setPositionAlignment(Qt::AlignHCenter|Qt::AlignBottom);
        lineText->position->setCoords(xStress[i],yStress[i]+1);
        lineText->setText(QString::number(yStress[i]));
        lineText->setTextAlignment(Qt::AlignCenter);
        lineText->setFont(lineFont);
        lineText->setPadding(QMargins(1, 1, 1, 1));

        QCPItemText *barText = new QCPItemText(ui->widget_plot);
        barText->position->setType(QCPItemPosition::ptPlotCoords);
        barText->position->setAxes(ui->widget_plot->xAxis,ui->widget_plot->yAxis2);
        barText->position->setCoords(xBar[i],yValues[i]/2);
        barText->setText(QString::number(yValues[i]));
        barText->setFont(barFont);
        barText->setColor(Qt::white);

        QCPItemText *workText = new QCPItemText(ui->widget_plot);
        workText->position->setType(QCPItemPosition::ptPlotCoords);
        workText->position->setCoords(xWorks[i],yWorks[i]/2);
        workText->setText(QString::number(yWorkCount[i]));
        workText->setFont(barFont);
        workText->setColor(Qt::red);
    }

    for(int i = 0; i < xLTS.count(); ++i)
    {
        QCPItemTracer *itemTracer = new QCPItemTracer(ui->widget_plot);
        itemTracer->setGraph(ltsLine);
        itemTracer->setGraphKey(xLTS[i]);
        itemTracer->setStyle(QCPItemTracer::tsCircle);
        itemTracer->setBrush(Qt::green);

        QCPItemText *ltsText = new QCPItemText(ui->widget_plot);
        ltsText->position->setType(QCPItemPosition::ptPlotCoords);
        ltsText->setPositionAlignment(Qt::AlignHCenter|Qt::AlignBottom);
        ltsText->position->setCoords(xLTS[i],yLTS[i]+1);
        ltsText->setText(QString::number(yLTS[i]));
        ltsText->setTextAlignment(Qt::AlignCenter);
        ltsText->setFont(lineFont);
        ltsText->setPadding(QMargins(1, 1, 1, 1));
    }

    QSharedPointer<QCPAxisTickerDateTime> dateTimeTicker(new QCPAxisTickerDateTime);
    dateTimeTicker->setDateTimeSpec(Qt::UTC);
    dateTimeTicker->setTickStepStrategy(QCPAxisTicker::tssMeetTickCount);
    dateTimeTicker->setDateTimeFormat("dd.MM");
    dateTimeTicker->setTickCount(dayCount);

    ui->widget_plot->xAxis->setRange(xRange);
    ui->widget_plot->xAxis->setTicker(dateTimeTicker);

    ui->widget_plot->replot();
    isLoad = true;
}

void week_popup::on_comboBox_yValue_currentIndexChanged(int index)
{
    if(isLoad) this->set_weekPlot(index);
}

void week_popup::on_toolButton_close_clicked()
{
    reject();
}

void week_popup::on_toolButton_edit_clicked()
{
    accept();
}
