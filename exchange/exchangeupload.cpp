#include "exchangeupload.h"
#include "qnet.h"
#include "cnfapp.h"
#include "sqlthread.h"
#include "cnfmaindb.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QDate>
#include <QJsonDocument>

ExchangeUpload::ExchangeUpload(QObject *parent) :
    QObject(parent)
{
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(timeout()));
#ifdef QT_DEBUG
    mTimer.start(2000);
#else
    mTimer.start(60000);
#endif
}

void ExchangeUpload::timeout()
{
    SqlThread *st = new SqlThread();
    connect(st, SIGNAL(finished()), this, SLOT(sqlFinished()));
    st->setDatabase(__cnfmaindb.fHost, __cnfmaindb.fDatabase, __cnfmaindb.fUser, __cnfmaindb.fPassword, "lc_ctype=utf8");
    st->setSqlQuery("select * from sys_data_exchange where fup is null");
    st->start();

    SqlThread *st2 = new SqlThread();
    connect(st2, SIGNAL(finished()), this, SLOT(uploadAmounts()));
    st2->setDatabase(__cnfmaindb.fHost, __cnfmaindb.fDatabase, __cnfmaindb.fUser, __cnfmaindb.fPassword, "lc_ctype=utf8");
    st2->bind(":date_cash", QDate::currentDate().addDays(-10));
    st2->setSqlQuery("select date_cash, count(id) as qty, sum(amount) as amount from o_order where (date_cash>:date_cash and state_id=2) or (state_id=1) group by 1");
    st2->start();
}

void ExchangeUpload::sqlFinished()
{
    SqlThread *st = static_cast<SqlThread*>(sender());
    QJsonArray ja;
    while (st->nextRow()) {
        QJsonObject j;
        j["id"] = st->getInteger("id");
        j["op"] = st->getInteger("fop");
        j["data"] = st->getString("fdata");
        ja.append(j);
    }
    QNet *n = new QNet();
    connect(n, SIGNAL(getResponse(QString,bool)), this, SLOT(getResponse(QString,bool)));
    n->URL = __cnfapp.exchangeServer() + "/exchange_up.php";
    n->addData("reqpass", __cnfapp.exchangePassword());
    n->addData("reqpass", __cnfapp.exchangePassword());
    n->addData("from", __cnfapp.exchangeIdentifyBy());
    n->addData("data", QJsonDocument(ja).toJson(QJsonDocument::Compact));
    n->go();
    st->deleteLater();
}

void ExchangeUpload::uploadAmounts()
{
    SqlThread *st = static_cast<SqlThread*>(sender());
    QJsonArray ja;
    while (st->nextRow()) {
        QJsonObject j;
        j["date"] = st->getDate("date_cash").toString("yyyy-MM-dd");
        j["qty"] = st->getInteger("qty");
        j["amount"] = st->getDouble("amount");
        ja.append(j);
    }
    QNet *n = new QNet();
    connect(n, SIGNAL(getResponse(QString,bool)), this, SLOT(getAmountResponse(QString,bool)));
    n->URL = __cnfapp.exchangeServer() + "/amounts_up.php";
    n->addData("reqpass", __cnfapp.exchangePassword());
    n->addData("reqpass", __cnfapp.exchangePassword());
    n->addData("from", __cnfapp.exchangeIdentifyBy());
    n->addData("data", QJsonDocument(ja).toJson(QJsonDocument::Compact));
    n->go();
    st->deleteLater();
}

void ExchangeUpload::getResponse(const QString &data, bool isError)
{
    sender()->deleteLater();
    if (!isError) {
        QJsonDocument jd = QJsonDocument::fromJson(data.toUtf8());
        QJsonArray ja = jd.array();
        for (int i = 0; i < ja.count(); i++) {
            QJsonObject jo = ja.at(i).toObject();
            QStringList keys = jo.keys();
            for (const QString &s: keys) {
                SqlThread *st = new SqlThread();
                connect(st, SIGNAL(finished()), st, SLOT(deleteLater()));
                st->setDatabase(__cnfmaindb.fHost, __cnfmaindb.fDatabase, __cnfmaindb.fUser, __cnfmaindb.fPassword, "lc_ctype=utf8");
                st->bind(":fid", s);
                st->bind(":fup", jo[s].toInt());
                st->setSqlQuery("update sys_data_exchange set fup=:fup where fid=:fid");
                st->start();
            }
        }
    }
}

void ExchangeUpload::getAmountResponse(const QString &data, bool isError)
{
    qDebug() << data;
    Q_UNUSED(data);
    Q_UNUSED(isError);
    sender()->deleteLater();
}
