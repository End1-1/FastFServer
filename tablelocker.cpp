#include "tablelocker.h"
#include "qsslserver.h"
#include "tableorder.h"
#include "dialog.h"
#include <QApplication>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QSslCipher>

TableLocker::TableLocker() :
    ThreadObject()
{
    fServer = new QSslServer(this);
    connect(this, SIGNAL(logMsg(QString)), __logDialog, SLOT(logMsg(QString)));
}

TableLocker::~TableLocker()
{
    fServer->close();
    delete fServer;
}

void TableLocker::run()
{
    QString dir = qApp->applicationDirPath() + "/";
    fServer->setSslLocalCertificate(dir + "cert.pem");
    fServer->setSslPrivateKey(dir + "key.pem");
    fServer->setSslProtocol(QSsl::TlsV1_2OrLater);
    fServer->listen(QHostAddress::Any, 8788);
    connect(fServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
}

void TableLocker::newConnection()
{
    QSslSocket *sslSocket = dynamic_cast<QSslSocket*>(fServer->nextPendingConnection());
    emit logMsg(QString("New connection from %1").arg(QHostAddress(sslSocket->peerAddress().toIPv4Address()).toString()));
    TableOrder *to = new TableOrder(sslSocket);
    sslSocket->setParent(to);
    connect(to, SIGNAL(logMsg(QString)), __logDialog, SLOT(logMsg(QString)));
    to->start();
}
