#include "cnfapp.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QHostInfo>
#include <QMessageBox>
#include <QSqlError>

#define TaxIP "tax_ip_address"
#define TaxPort "tax_port"
#define TaxPassword "tax_password"
#define TaxDept "tax_dept"

#define OnlineReportAddress "online_report_address"
#define OnlineReportPassword "online_report_password"
#define OnlineReportCafeId "online_report_cafe_id"

QMap<QString, QString> CnfApp::fData;
CnfApp __cnfapp;

CnfApp::CnfApp()
{

}

QString CnfApp::taxIP()
{
    return fData[TaxIP];
}

int CnfApp::taxPort()
{
    return fData[TaxPort].toInt();
}

QString CnfApp::taxPassword()
{
    return fData[TaxPassword];
}

QString CnfApp::taxDept()
{
    return fData[TaxDept];
}

QString CnfApp::onlineReportAddress()
{
    return fData[OnlineReportAddress];
}

QString CnfApp::onlineReportPassword()
{
    return fData[OnlineReportPassword];
}

QString CnfApp::onlineReportCafeId()
{
    return fData[OnlineReportCafeId];
}

void CnfApp::init(const QString &dbHost, const QString &dbPath, const QString &dbUser, const QString &dbPass, const QString &prefix)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QIBASE", "CnfApp");
    db.setHostName(dbHost);
    db.setDatabaseName(dbPath);
    db.setUserName(dbUser);
    db.setPassword(dbPass);
    if (!db.open()) {
        QMessageBox::critical(nullptr, "Db error", db.lastError().databaseText());
        return;
    }
    QString settingsName = prefix.toUpper() + "-" + QHostInfo::localHostName().toUpper();
    QSqlQuery q(db);
    q.prepare("select fkey, fvalue from sys_cnfapp where fhost=:fhost");
    q.bindValue(":fhost", settingsName);
    q.exec();
    while (q.next()) {
        fData[q.value(0).toString()] = q.value(1).toString();
    }
    if (!fData.contains(TaxIP)) {
        QStringList v;
        v << TaxIP << TaxPort << TaxPassword << TaxDept;
        foreach (QString s, v) {
            q.prepare("insert into sys_cnfapp (fhost, fkey) values (:fhost, :fkey)");
            q.bindValue(":fhost", settingsName);
            q.bindValue(":fkey", s);
            q.exec();
        }
    }
    if (!fData.contains(OnlineReportAddress)) {
        QStringList v;
        v << OnlineReportAddress << OnlineReportCafeId << OnlineReportPassword;
        foreach (QString s, v) {
            q.prepare("insert into sys_cnfapp (fhost, fkey) values (:fhost, :fkey)");
            q.bindValue(":fhost", settingsName);
            q.bindValue(":fkey", s);
            q.exec();
        }
    }
    db.close();
    QSqlDatabase::removeDatabase("CnfApp");
}
