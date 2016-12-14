#ifndef SUMMERY_DELEGATE
#define SUMMERY_DELEGATE

#include <QtGui>
#include <QItemDelegate>
#include <QTableView>
#include "settings.h"

class summery_delegate : public QItemDelegate
{
    Q_OBJECT

public:
    summery_delegate(QTableView *parent = 0) : QItemDelegate(parent) {}

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QVector<int> fontsize = settings::get_fontSize();
        QFont phase_font,date_font, work_font;
        QString temp_value;
        QStringList sum_values;
        QString delimiter = "-";
        QColor rect_color;
        int textMargin = 2;
        phase_font.setBold(true);
        phase_font.setPixelSize(fontsize[0]);
        date_font.setBold(true);
        date_font.setPixelSize(fontsize[1]);
        work_font.setBold(false);
        work_font.setPixelSize(fontsize[2]);

        temp_value = index.data(Qt::DisplayRole).toString();
        sum_values = temp_value.split(delimiter);

        for(int i = 0; i < settings::get_sportList().count(); ++i)
        {
            if(sum_values.at(0) == settings::get_sportList().at(i))
            {
                rect_color = settings::get_color(settings::get_sportColor().at(i));
                break;
            }
            else
            {
                //Summery
                rect_color.setRgb(0,255,255);
            }
        }

        QRect rect_head(option.rect.x(),option.rect.y(),option.rect.width(),20);
        QRect rect_head_text(option.rect.x()+textMargin,option.rect.y(),option.rect.width(),20);
        painter->setBrush(QBrush(rect_color));
        painter->fillRect(rect_head,QBrush(rect_color));
        painter->fillRect(rect_head_text,QBrush(rect_color));
        painter->drawRect(rect_head);
        QTextOption headoption(Qt::AlignLeft);
        painter->setPen(Qt::black);
        painter->setFont(date_font);
        painter->drawText(rect_head_text,sum_values.at(0),headoption);

        QString labels;
        labels = "Workouts:\n";
        labels = labels + "Duration(Hours):\n";
        labels = labels + "Amount(%):\n";
        labels = labels + "Distance(Km):\n";
        labels = labels + "Stress(TSS):";

        QRect rect_label(option.rect.x(),option.rect.y()+21,option.rect.width()/2,option.rect.height()-21);
        QRect rect_label_text(option.rect.x()+textMargin,option.rect.y()+21,option.rect.width()/2,option.rect.height()-21);
        painter->setBrush(QBrush(rect_color));
        painter->fillRect(rect_label,QBrush(rect_color));
        painter->fillRect(rect_label_text,QBrush(rect_color));
        QTextOption labeloption(Qt::AlignLeft);
        painter->setPen(Qt::black);
        painter->setFont(work_font);
        painter->drawText(rect_label_text,labels,labeloption);

        if(!sum_values.isEmpty())
        {

            QString partValue;
            partValue = sum_values.at(1) + "\n";
            partValue = partValue + sum_values.at(2) + "\n";
            partValue = partValue + sum_values.at(3) + "\n";
            partValue = partValue + sum_values.at(4) + "\n";
            partValue = partValue + sum_values.at(5) + "\n";

            QRect rect_work(option.rect.x()+(option.rect.width()/2) ,option.rect.y()+21,(option.rect.width()/2)+1,option.rect.height()-21);
            painter->setBrush(QBrush(rect_color));
            painter->fillRect(rect_work,QBrush(rect_color));
            QTextOption workoption(Qt::AlignLeft);
            painter->setPen(Qt::black);
            painter->setFont(work_font);
            painter->drawText(rect_work,partValue,workoption);
        }
        painter->restore();
    }
};

#endif // SUMMERY_DELEGATE

