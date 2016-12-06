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

#include <QDebug>
#include <QClipboard>
#include "dialog_pacecalc.h"
#include "ui_dialog_pacecalc.h"

Dialog_paceCalc::Dialog_paceCalc(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_paceCalc)
{
    ui->setupUi(this);
    sportList << settings::isSwim << settings::isBike << settings::isRun;
    model_header << "Distance" << "Duration";
    dist <<25<<50<<100<<200<<300<<400<<500<<600<<800<<1000;
    distFactor = 1;
    pace_model = new QStandardItemModel();
    this->init_paceView();
    ui->comboBox_sport->addItems(sportList);
}

Dialog_paceCalc::~Dialog_paceCalc()
{
    delete ui;
}

void Dialog_paceCalc::on_pushButton_close_clicked()
{
    delete pace_model;
    reject();
}

void Dialog_paceCalc::init_paceView()
{
    pace_model->setHorizontalHeaderLabels(model_header);
    ui->tableView_pace->setModel(pace_model);
    ui->tableView_pace->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_pace->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_pace->verticalHeader()->hide();


    for(int i = 0; i < 10; ++i)
    {
        pace_model->insertRows(i,1,QModelIndex());
        pace_model->setData(pace_model->index(i,0,QModelIndex()),dist[i]);
    }
}

void Dialog_paceCalc::set_pace()
{
    int pace = settings::get_timesec(ui->timeEdit_pace->time().toString("mm:ss"));

    for(int i = 0; i < 10; ++i)
    {
        if(ui->comboBox_sport->currentText() == settings::isSwim)
        {
            pace_model->setData(pace_model->index(i,0,QModelIndex()),dist[i]*distFactor);
            pace_model->setData(pace_model->index(i,1,QModelIndex()),settings::set_time(static_cast<int>(round(pace * (dist[i]*distFactor)/100))));
        }
        else
        {
            pace_model->setData(pace_model->index(i,0,QModelIndex()),dist[i]*distFactor);
            pace_model->setData(pace_model->index(i,1,QModelIndex()),settings::set_time(static_cast<int>(round(pace * (dist[i]*distFactor)/1000))));
        }
    }
}

void Dialog_paceCalc::set_freeField(int dist)
{
    int pace = settings::get_timesec(ui->timeEdit_pace->time().toString("mm:ss"));

    if(ui->comboBox_sport->currentText() == settings::isSwim)
    {
        ui->lineEdit_dura->setText(settings::set_time(pace * dist/100));
    }
    else
    {
        ui->lineEdit_dura->setText(settings::set_time(pace * dist/1000));
    }
}

void Dialog_paceCalc::on_comboBox_sport_currentTextChanged(const QString &sport)
{
    if(sport == settings::isSwim)
    {
        ui->label_pace->setText("/100m");
    }
    else
    {
        ui->label_pace->setText("/km");
    }

    this->set_pace();
    ui->lineEdit_speed->setText(settings::get_speed(ui->timeEdit_pace->time(),0,sport,true));
    this->set_freeField(ui->lineEdit_dist->text().toInt());
}


void Dialog_paceCalc::on_timeEdit_pace_timeChanged(const QTime &time)
{
    ui->lineEdit_speed->setText(settings::get_speed(time,0,ui->comboBox_sport->currentText(),true));
    this->set_freeField(ui->lineEdit_dist->text().toInt());
    this->set_pace();
}

void Dialog_paceCalc::on_lineEdit_dist_textChanged(const QString &dist)
{
    this->set_freeField(dist.toInt());
    ui->lineEdit_distkm->setText(QString::number(dist.toDouble()/1000)+" Km");
}

void Dialog_paceCalc::on_spinBox_factor_valueChanged(int value)
{
    distFactor = value;
    this->set_pace();
}

void Dialog_paceCalc::on_timeEdit_intTime_timeChanged(const QTime &time)
{
    ui->lineEdit_IntSpeed->setText(settings::get_speed(time,ui->spinBox_IntDist->value(),ui->comboBox_sport->currentText(),false));
}

void Dialog_paceCalc::on_spinBox_IntDist_valueChanged(int dist)
{
    ui->lineEdit_IntSpeed->setText(settings::get_speed(ui->timeEdit_intTime->time(),dist,ui->comboBox_sport->currentText(),false));
}

void Dialog_paceCalc::on_pushButton_clicked()
{
    ui->comboBox_sport->setCurrentIndex(0);
    ui->spinBox_factor->setValue(1);
    ui->timeEdit_pace->setTime(QTime::fromString("00:00","mm:ss"));
    ui->timeEdit_intTime->setTime(QTime::fromString("00:00","mm:ss"));
    ui->spinBox_IntDist->setValue(1);
    ui->lineEdit_IntSpeed->setText("");
}

void Dialog_paceCalc::on_pushButton_copy_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    QByteArray speedArray;
    QMimeData *mimeData = new QMimeData();
    double speed = ui->lineEdit_IntSpeed->text().toDouble();
    int sec = settings::get_timesec(ui->timeEdit_intTime->time().toString("mm:ss"));

    for(int i = 0; i < sec; ++i)
    {
        speedArray.append(QString::number(speed));
        speedArray.append("\r\n");
    }
    mimeData->setData("text/plain",speedArray);
    clipboard->setMimeData(mimeData);
}
