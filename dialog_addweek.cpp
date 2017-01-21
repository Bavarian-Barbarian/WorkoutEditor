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

#include "dialog_addweek.h"
#include "ui_dialog_addweek.h"

Dialog_addweek::Dialog_addweek(QWidget *parent, QString sel_week, schedule *p_sched) :
    QDialog(parent),
    ui(new Ui::Dialog_addweek)
{
    ui->setupUi(this);
    workSched = p_sched;
    metaProxy = new QSortFilterProxyModel();
    metaProxy->setSourceModel(p_sched->week_meta);
    contentProxy = new QSortFilterProxyModel();
    contentProxy->setSourceModel(p_sched->week_content);

    ui->comboBox_phase->addItems(settings::get_listValues("Phase"));
    ui->comboBox_cycle->addItems(settings::get_listValues("Cycle"));
    ui->dateEdit_selectDate->setDate(QDate().currentDate());
    timeFormat = "hh:mm:ss";
    empty = "0-0-00:00-0";
    weekHeader << "Sport" << "Workouts" << "Duration" << "%" << "Distance" << "Pace" << "Stress";
    sportuseList = settings::get_listValues("Sportuse");
    this->setFixedHeight(100+(35*(sportuseList.count()+1)));
    this->setFixedWidth(650);
    this->fill_values(sel_week);
}

Dialog_addweek::~Dialog_addweek()
{
    delete ui;
    delete weekModel;
}

void Dialog_addweek::on_pushButton_cancel_clicked()
{
    reject();
}

void Dialog_addweek::fill_values(QString selWeek)
{
    QStringList weekInfo = selWeek.split("-");
    metaProxy->setFilterRegExp("\\b"+weekInfo.at(1)+"\\b");
    metaProxy->setFilterKeyColumn(1);
    contentProxy->setFilterRegExp("\\b"+weekInfo.at(1)+"\\b");
    contentProxy->setFilterKeyColumn(1);

    QTime duration;
    QString value,work,dura,dist,stress;
    QString sumString = settings::get_generalValue("sum");
    QStringList values;
    int listCount = sportuseList.count();

    weekModel = new QStandardItemModel(sportuseList.count()+1,7);
    weekModel->setHorizontalHeaderLabels(weekHeader);
    ui->tableView_sportValues->setModel(weekModel);
    ui->tableView_sportValues->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_sportValues->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_sportValues->verticalHeader()->hide();
    ui->tableView_sportValues->setItemDelegate(&week_del);

    QAbstractItemModel *ab_model = ui->tableView_sportValues->model();

    if(metaProxy->rowCount() > 0)
    {
        openID = weekInfo.at(1);
        ui->dateEdit_selectDate->setDate(QDate::fromString(metaProxy->data(metaProxy->index(0,3)).toString(),"dd.MM.yyyy"));
        ui->lineEdit_week->setText(QString::number(ui->dateEdit_selectDate->date().weekNumber()));
        value = weekInfo.at(3);
        ui->comboBox_phase->setCurrentText(value.split("_").first());
        ui->comboBox_cycle->setCurrentText(value.split("_").last());
        update = true;
    }
    else
    {
        update = false;
    }

    if(contentProxy->rowCount() > 0)
    {
        for(int i = 2,row = 0; i < listCount+2; ++i,++row)
        {            
            value = contentProxy->data(contentProxy->index(0,i)).toString();
            values = value.split("-");
            work = values.at(0);
            dist = values.at(1);
            dura = values.at(2);
            stress = values.at(3);
            duration = QTime::fromString(dura,"hh:mm");

            weekModel->setData(weekModel->index(row,0,QModelIndex()),sportuseList.at(row));
            weekModel->setData(weekModel->index(row,1,QModelIndex()),work.toInt());
            weekModel->setData(weekModel->index(row,2,QModelIndex()),duration);
            weekModel->setData(weekModel->index(row,3,QModelIndex()),0.0);
            weekModel->setData(weekModel->index(row,4,QModelIndex()),this->set_doubleValue(dist.toDouble(),false));
            weekModel->setData(weekModel->index(row,5,QModelIndex()),this->get_workout_pace(dist.toDouble(),duration,sportuseList.at(row),false));
            weekModel->setData(weekModel->index(row,6,QModelIndex()),stress.toInt());        
        }

        weekModel->setData(weekModel->index(listCount,0,QModelIndex()),sumString);
        weekModel->setData(weekModel->index(listCount,1,QModelIndex()),week_del.sum_int(ab_model,&sportuseList,1));
        weekModel->setData(weekModel->index(listCount,2,QModelIndex()),week_del.sum_time(ab_model,&sportuseList,2));
        weekModel->setData(weekModel->index(listCount,3,QModelIndex()),100);
        weekModel->setData(weekModel->index(listCount,4,QModelIndex()),this->set_doubleValue(week_del.sum_double(ab_model,&sportuseList,4),false));
        weekModel->setData(weekModel->index(listCount,5,QModelIndex()),"--");
        weekModel->setData(weekModel->index(listCount,6,QModelIndex()),week_del.sum_int(ab_model,&sportuseList,6));
    }
    else
    {
        for(int row = 0; row < listCount; ++row)
        {
            weekModel->setData(weekModel->index(row,0,QModelIndex()),sportuseList.at(row));
            weekModel->setData(weekModel->index(row,1,QModelIndex()),0);
            weekModel->setData(weekModel->index(row,2,QModelIndex()),QTime::fromString("00:00:00"));
            weekModel->setData(weekModel->index(row,3,QModelIndex()),0.0);
            weekModel->setData(weekModel->index(row,4,QModelIndex()),0.0);
            weekModel->setData(weekModel->index(row,5,QModelIndex()),"--");
            weekModel->setData(weekModel->index(row,6,QModelIndex()),0);
        }
        weekModel->setData(weekModel->index(listCount,0,QModelIndex()),sumString);
    }

    week_del.calc_percent(&sportuseList,ab_model);
    metaProxy->setFilterRegExp("");
    contentProxy->setFilterRegExp("");
}

void Dialog_addweek::store_values()
{
    weekMeta = QStringList();
    weekID = ui->lineEdit_week->text()+"_"+selYear;
    int currID = metaProxy->rowCount()+1;
    if(update)
    {
        weekMeta << weekID
                 << ui->comboBox_phase->currentText()+"_"+ui->comboBox_cycle->currentText()
                 << ui->dateEdit_selectDate->date().toString("dd.MM.yyyy");

        weekContent << weekID
                    << this->create_values();
    }
    else
    {
        weekMeta << QString::number(currID)
                 << weekID
                 << ui->comboBox_phase->currentText()+"_"+ui->comboBox_cycle->currentText()
                 << ui->dateEdit_selectDate->date().toString("dd.MM.yyyy");

        weekContent << QString::number(currID)
                    << weekID
                    << this->create_values();
    }
}

QStringList Dialog_addweek::create_values()
{
    QString splitter = "-",vString;
    QStringList list;

    for(int i = 0; i < sportuseList.count()+1; ++i)
    {
        vString = weekModel->data(weekModel->index(i,1,QModelIndex())).toString()+splitter;
        vString = vString+weekModel->data(weekModel->index(i,4,QModelIndex())).toString()+splitter;
        vString = vString+weekModel->data(weekModel->index(i,2,QModelIndex())).toTime().toString("hh:mm")+splitter;
        vString = vString+weekModel->data(weekModel->index(i,6,QModelIndex())).toString();
        list << vString;
    }
    return list;
}

void Dialog_addweek::on_dateEdit_selectDate_dateChanged(const QDate &date)
{
    ui->lineEdit_week->setText(QString::number(date.weekNumber()));
    selYear = QString::number(date.year());
}

void Dialog_addweek::on_pushButton_ok_clicked()
{
    this->store_values();

    if(update)
    {
        metaProxy->setFilterRegExp("\\b"+openID+"\\b");
        metaProxy->setFilterKeyColumn(1);
        contentProxy->setFilterRegExp("\\b"+openID+"\\b");
        contentProxy->setFilterKeyColumn(1);

        if(metaProxy->rowCount() > 0)
        {
            for(int i = 0; i < weekMeta.count(); ++i)
            {
                metaProxy->setData(metaProxy->index(0,i+1),weekMeta.at(i));
            }
        }

        if(contentProxy->rowCount() > 0)
        {
            for(int i = 1; i <= weekContent.count(); ++i)
            {
                contentProxy->setData(contentProxy->index(0,i),weekContent.at(i-1));
            }
        }
        metaProxy->setFilterRegExp("");
        contentProxy->setFilterRegExp("");
    }
    else
    {
        int rowcount;
        rowcount = metaProxy->rowCount();
        metaProxy->insertRow(rowcount,QModelIndex());

        for(int i = 0; i < weekMeta.count(); ++i)
        {
            metaProxy->setData(metaProxy->index(rowcount,i,QModelIndex()),weekMeta.at(i));
        }

        rowcount = contentProxy->rowCount();
        contentProxy->insertRow(rowcount,QModelIndex());

        for(int i = 0; i < weekContent.count(); ++i)
        {
            contentProxy->setData(contentProxy->index(rowcount,i,QModelIndex()),weekContent.at(i));
        }
    }
    workSched->set_isUpdated(true);
    accept();
}
