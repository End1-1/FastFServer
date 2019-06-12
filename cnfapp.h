#ifndef CNFAPP_H
#define CNFAPP_H

#include <QObject>

#define PaymentCash 1
#define PaymentArca 2
#define PaymentMaster 3
#define PaymentVisa 4
#define PaymentMaestro 5
#define PaymentIdram 6
#define PaymentJazzveGift 7

#define PaymentServiceDiscount 1
#define PaymentNoServiceDiscount 2

class CnfApp
{
    static QMap<QString, QString> fData;
public:
    CnfApp();    
    static QString taxIP();
    static int taxPort();
    static QString taxPassword();
    static QString taxDept();
    static QString onlineReportAddress();
    static QString onlineReportPassword();
    static QString onlineReportCafeId();
    static void init(const QString &dbHost, const QString &dbPath, const QString &dbUser, const QString &dbPass, const QString &prefix);
};

extern CnfApp __cnfapp;

#endif // CNFAPP_H
