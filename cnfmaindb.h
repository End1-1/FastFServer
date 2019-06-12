#ifndef CNFMAINDB_H
#define CNFMAINDB_H

#include <QString>

class CnfMainDb
{
public:
    CnfMainDb();
    static bool fOk;
    static QString fHost;
    static QString fDatabase;
    static QString fUser;
    static QString fPassword;
    static QString fLastUsername;
    static QString fLastDatabase;
    static QString fServerIP;
    static QString fServerMode;
    static QString fTaxDepartment;
    static void write();
};

extern CnfMainDb __cnfmaindb;

#endif // CNFMAINDB_H
