#ifndef MJSONHANDLER_H
#define MJSONHANDLER_H

#include <QObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QHostAddress>
#include "msqldatabase.h"

class MJsonHandler : public QObject
{
    Q_OBJECT
    QHostAddress fClientAddr;
    static QMap<QString, int> fSessions; //session - user
    static QMap<int, QString> fLockTables; // tableid - session
    QByteArray handleDish(const QJsonObject &o);
    QByteArray handleDishType(const QJsonObject &o);
    QByteArray handleHall(const QJsonObject &o);
    QByteArray handleQuery(const QJsonObject &o);
    QByteArray handleServer(const QJsonObject &o);
    QByteArray handleLockTable(const QJsonObject &o);
    QByteArray handleUnlockTable(const QJsonObject &o);
    QByteArray handleDishToOrder(const QJsonObject &o);
    QByteArray handleOpenTable(const QJsonObject &o);
    QByteArray handleLoadOrder(const QJsonObject &o);
    QByteArray handleDishChange(const QJsonObject &o);
    QByteArray handleDishRemove(const QJsonObject &o);
    QByteArray handlePrintKitchen(const QJsonObject &o);
    QByteArray handleReceipt(const QJsonObject &o);
    QByteArray handleDishInfo(const QJsonObject &o);
    QByteArray handleSettings(const QJsonObject &o);
    QByteArray handleListReceipt(const QJsonObject &o);
    QByteArray handleCloseOrder(const QJsonObject &o);
    QByteArray handleDishComment(const QJsonObject &o);
    QByteArray handleCommentsList(const QJsonObject &o);
    QByteArray handleReadyDishes(const QJsonObject &o);
    QByteArray handleReadyDishDone(const QJsonObject &o);
    QByteArray handleMyMoney(const QJsonObject &o);
    QByteArray handleTaxReport(const QJsonObject &o);
    QByteArray handleTaxCancel(const QJsonObject &o);
    QString updateOrderAmount(const QString &id);
    int checkPassword(const QString &pwd);
    QString sessionNew();
    QByteArray jsonError(const QString &err);

public:
    explicit MJsonHandler(const QHostAddress &clientAddr, QObject *parent = nullptr);
    static QString fServerIp;
    QByteArray handle(const QByteArray &rawData);

private:
    QString fError;
    MSqlDatabase fDb;

signals:

public slots:
};

#endif // MJSONHANDLER_H
