#include "tableorder.h"
#include "fastfprotocol.h"
#include "dialog.h"
#include "cnfmaindb.h"
#include <QMutex>

static QMap<int, int> fTables;
static QMutex fMutex;

TableOrder::TableOrder(QSslSocket *sslSocket) :
    ThreadObject(),
    fSocket(sslSocket)
{
    fSocketDescriptor = fSocket->socketDescriptor();
    fSqlThread = new SqlThread();
    fSqlThread->setDatabase(__cnfmaindb.fHost, __cnfmaindb.fDatabase, __cnfmaindb.fUser, __cnfmaindb.fPassword, "lc_ctype=utf8");
    //connect(&fSqlThread, SIGNAL(finished()), this, SLOT(queryReady()));
}

TableOrder::~TableOrder()
{
    unlockTable();
    delete fSocket;
    delete fSqlThread;
}

void TableOrder::run()
{
    reset();
    connect(fSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(fSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
}

void TableOrder::reset()
{
    fDataSize = 0;
    fDataType = 0;
    fDataRead = 0;
    fData.clear();
}

void TableOrder::lockTable()
{
    QMutexLocker ml(&fMutex);
    int table = fData.toInt();
    QList<int> tables = fTables.values();
    ml.unlock();
    quint32 dataSize = sizeof(quint32) * 1;
    quint32 dataType = p_locktable;
    quint32 data = 0;
    if (tables.contains(table)) {
        fSocket->write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
        fSocket->write(reinterpret_cast<const char*>(&dataType), sizeof(dataType));
        fSocket->write(reinterpret_cast<const char*>(&data), sizeof(data));
        fSocket->waitForBytesWritten();
        fSocket->disconnectFromHost();
        emit logMsg("Lock table " + QString::number(table) + " failed");
        return;
    }
    ml.relock();
    fTables[fSocketDescriptor] = table;
    ml.unlock();
    data = 1;
    emit logMsg("Lock table " + QString::number(table) + " success");
    fSocket->write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
    fSocket->write(reinterpret_cast<const char*>(&dataType), sizeof(dataType));
    fSocket->write(reinterpret_cast<const char*>(&data), sizeof(data));
    fSocket->waitForBytesWritten();
}

void TableOrder::lockTableByOrder()
{
    fSqlThread->bind(":id", fData);
    fSqlThread->setSqlQuery("select table_id from o_order where id=:id");
    fSqlThread->run();
    if (fSqlThread->nextRow()) {
        fData = fSqlThread->getString("TABLE_ID").toUtf8();
        lockTable();
    } else {
        quint32 dataSize = sizeof(quint32) * 1;
        quint32 dataType = p_locktable_order;
        quint32 data = 0;
        fSocket->write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
        fSocket->write(reinterpret_cast<const char*>(&dataType), sizeof(dataType));
        fSocket->write(reinterpret_cast<const char*>(&data), sizeof(data));
        fSocket->waitForBytesWritten();
        fSocket->disconnectFromHost();
        emit logMsg("Lock table by order " + fData + " failed");
        return;
    }
}

void TableOrder::unlockTable()
{
    QMutexLocker ml(&fMutex);
    if (fTables.contains(fSocketDescriptor)) {
        emit logMsg("Unlock table " + QString::number(fTables[fSocketDescriptor]) + " success");
        fTables.remove(fSocketDescriptor);
    }
    ml.unlock();
}

void TableOrder::readyRead()
{
    if (fDataSize == 0) {
        fSocket->read(reinterpret_cast<char *>(&fDataSize), sizeof(fDataSize));
    }
    if (fDataType == 0) {
        fSocket->read(reinterpret_cast<char *>(&fDataType), sizeof(fDataType));
    }
    fData.append(fSocket->read(fDataSize - fDataRead));
    fDataRead = fData.size();
    if (fDataRead == fDataSize) {
        switch (fDataType) {
        case p_locktable:
            lockTable();
            break;
        case p_locktable_order:
            lockTableByOrder();
            break;
        }
        reset();
    }
}

void TableOrder::disconnected()
{
    emit finished();
    deleteLater();
}
