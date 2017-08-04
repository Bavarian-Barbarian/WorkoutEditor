#ifndef SAISONS_H
#define SAISONS_H

#include <QStandardItemModel>
#include <QtXml>
#include "settings.h"
#include "xmlhandler.h"


class saisons : public xmlHandler
{
public:
    saisons();
    QStandardItemModel *saisonsModel,*contestModel;
    void update_saison(bool,int,QString,QDate,QDate,int);
    void remove_saison(int);
    QString get_currSaison() {return currSaison;}
    QVariant get_saisonInfo(QString,QString);

    void set_selSaison(QString value) {selSaison = value;}
    QString get_selSaison() {return selSaison;}
    QString saison_atDate(QDate);
    void write_saisonInfo();

private:
    QStringList saison_tags,contest_tags;
    QString saisonPath,saisonFile,currSaison,selSaison;
    QHash<QString,QMap<QString,QVariant> > saisonMap;

    void read_saisonInfo(QDomDocument);
    void fill_saisonMap();
};

#endif // SAISONS_H
