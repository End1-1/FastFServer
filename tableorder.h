#ifndef TABLEORDER_H
#define TABLEORDER_H

#include "threadobject.h"
#include "sqlthread.h"
#include <QSslSocket>

class TableOrder : public ThreadObject
{
    Q_OBJECT
public:
    explicit TableOrder(QSslSocket *sslSocket);
    ~TableOrder();

protected slots:
    virtual void run() override;

private:
    QSslSocket *fSocket;
    SqlThread *fSqlThread;
    quint32 fDataSize;
    quint32 fDataRead;
    quint32 fDataType;
    QByteArray fData;
    int fSocketDescriptor;
    void reset();
    void lockTable();
    void lockTableByOrder();
    void unlockTable();

private slots:
    void readyRead();
    void disconnected();

signals:
    void logMsg(const QString &msg);
};

#endif // TABLEORDER_H
