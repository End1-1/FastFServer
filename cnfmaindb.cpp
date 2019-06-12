#include "cnfmaindb.h"
#include <QSettings>
#include <QMessageBox>

bool CnfMainDb::fOk;
QString CnfMainDb::fHost;
QString CnfMainDb::fDatabase;
QString CnfMainDb::fUser;
QString CnfMainDb::fPassword;
QString CnfMainDb::fLastUsername;
QString CnfMainDb::fLastDatabase;
QString CnfMainDb::fServerIP;
QString CnfMainDb::fServerMode;
QString CnfMainDb::fTaxDepartment;

CnfMainDb __cnfmaindb;

#define db_ver 2

CnfMainDb::CnfMainDb()
{
    fOk = false;
    QSettings s(_ORGANIZATION_, _APPLICATION_);
    if (s.value("db_ver").toInt() == 0) {
        s.setValue("db_ver", db_ver);
    }
    if (s.value("db_ver").toInt() != db_ver) {
        return;
    }
    fOk = true;
    QByteArray buf = s.value("db").toByteArray();
    for (int i = 0; i < buf.length(); i++) {
        buf[i] = buf[i] ^ ((i % 2) + (i % 3) + (i % 4) + (i % 5) + (i % 6) + (i % 7) + (i % 8) + (i % 9));
    }
    QList<QByteArray> params = buf.split('\r');
    if (params.count() < 9) {
        //QMessageBox::critical(0, "", "Invalid params count");
        return;
    }
    fHost = params.at(0);
    fDatabase = params.at(1);
    fUser = params.at(2);
    fPassword = params.at(3);
    fLastUsername = params.at(4);
    fLastDatabase = params.at(5);
    fServerIP = params.at(6);
    fServerMode = params.at(7);
    fTaxDepartment = params.at(8);
}

void CnfMainDb::write()
{
    QByteArray buf;
    buf.append(fHost);
    buf.append('\r');
    buf.append(fDatabase);
    buf.append('\r');
    buf.append(fUser);
    buf.append('\r');
    buf.append(fPassword);
    buf.append('\r');
    buf.append(fLastUsername);
    buf.append('\r');
    buf.append(fLastDatabase);
    buf.append('\r');
    buf.append(fServerIP);
    buf.append('\r');
    buf.append(fServerMode);
    buf.append('\r');
    buf.append(fTaxDepartment);
    buf.append('\r');
    for (int i = 0; i < buf.length(); i++) {
        buf[i] = buf[i] ^ ((i % 2) + (i % 3) + (i % 4) + (i % 5) + (i % 6) + (i % 7) + (i % 8) + (i % 9));
    }
    QSettings s(_ORGANIZATION_, _APPLICATION_);
    s.setValue("db", buf);
    s.setValue("db_ver", db_ver);
    fOk = true;
}
