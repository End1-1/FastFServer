#include "exchangedownload.h"
#include "qnet.h"
#include "sqlthread.h"
#include "cnfmaindb.h"
#include "cnfapp.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

ExchangeDownload::ExchangeDownload(QObject *parent) :
    QObject(parent)
{
    connect(&mTimer, &QTimer::timeout, this, &ExchangeDownload::timeout);
    mTimer.start(2000);
}

void ExchangeDownload::timeout()
{
    mTimer.stop();
    SqlThread *st = new SqlThread();
    connect(st, SIGNAL(finished()), this, SLOT(sql1Finished()));
    st->setDatabase(__cnfmaindb.fHost, __cnfmaindb.fDatabase, __cnfmaindb.fUser, __cnfmaindb.fPassword, "lc_ctype=utf8");
    st->setSqlQuery("select max(fexchangeid) as maxid from sys_data_exchange ");
    st->start();
}

void ExchangeDownload::sql1Finished()
{
    SqlThread *st = static_cast<SqlThread*>(sender());
    QString maxid = "0";
    if (st->nextRow()) {
        maxid = st->getString("maxid");
    }
    st->deleteLater();
    QNet *n = new QNet();
    connect(n, SIGNAL(getResponse(QString,bool)), this, SLOT(getResponse(QString,bool)));
    n->URL = __cnfapp.exchangeServer() + "/exchange_down.php";
    n->addData("reqpass", __cnfapp.exchangePassword());
    n->addData("reqpass", __cnfapp.exchangePassword());
    n->addData("max", maxid);
    n->go();
}

void ExchangeDownload::getResponse(const QString &data, bool isError)
{
    sender()->deleteLater();
    if (!isError) {
        QJsonDocument jd = QJsonDocument::fromJson(data.toUtf8());
        QJsonArray ja = jd.array();
        for (int i = 0; i < ja.count(); i++) {
            QJsonObject jo = ja.at(i).toObject();
            SqlThread *st = new SqlThread();
            connect(st, SIGNAL(finished()), st, SLOT(deleteLater()));
            st->setDatabase(__cnfmaindb.fHost, __cnfmaindb.fDatabase, __cnfmaindb.fUser, __cnfmaindb.fPassword, "lc_ctype=utf8");
            st->setSqlQuery("insert into sys_data_exchange (fcreateat, fcreatefrom, fop, fdata, fup, fexchangeid) values (current_timestamp, 'SERVER', :fop, :fdata, -1, :fexchangeid)");
            st->bind(":fop", jo["op"].toInt());
            st->bind(":fdata", QJsonDocument(jo).toJson(QJsonDocument::Compact));
            st->bind(":fexchangeid", jo["id"].toInt());
            st->start();
            switch (jo["op"].toInt()) {
            case 1: {
                jd = QJsonDocument::fromJson(jo["data"].toString().toUtf8());
                QJsonObject jc = jd.object();
                st = new SqlThread();
                connect(st, SIGNAL(finished()), st, SLOT(deleteLater()));
                st->setDatabase(__cnfmaindb.fHost, __cnfmaindb.fDatabase, __cnfmaindb.fUser, __cnfmaindb.fPassword, "lc_ctype=utf8");
                st->setSqlQuery("update costumers_names set card_value=:card_value where id=:id");
                st->bind(":id", jc["card"].toString().toInt());
                st->bind(":card_value", jc["card_value"].toDouble());
                st->start();
                break;
            }
            }
        }
    }
    mTimer.start(2000);
}
