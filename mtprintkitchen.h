#ifndef MTPRINTKITCHEN_H
#define MTPRINTKITCHEN_H

#include "mdefined.h"
#include "pprintscene.h"
#include <QThread>

class MTPrintKitchen : public QThread
{
    Q_OBJECT
public:
    MTPrintKitchen(const QMap<QString, QString> &data, const QList<QMap<QString, QString> > &dishes, bool kitchen, QObject *parent = nullptr);
    ~MTPrintKitchen();

protected:
    virtual void run();
    void printKitchen();
    void printReceipt();

private:
    bool fKitchen;
    QStringList fLog;
    QMap<QString, QString> fData;
    QList<QMap<QString, QString> > fDishes;
    PPrintScene *checkTop(int &top, PPrintScene *ps, QList<PPrintScene *> &lps);
};

#endif // MTPRINTKITCHEN_H
