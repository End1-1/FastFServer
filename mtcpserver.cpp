#include "mtcpserver.h"
#include "mdefined.h"
#include "mjsonhandler.h"
#include <QTcpSocket>
#include <QHostAddress>
#include <QMessageBox>
#include <QNetworkProxy>
#include <QApplication>

MTcpServer::MTcpServer(QObject *parent) :
    QThread(parent)
{
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
    fServer = new QTcpServer(this);
    fServer->setProxy(QNetworkProxy::NoProxy);
    connect(fServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
    connect(fServer, SIGNAL(acceptError(QAbstractSocket::SocketError)), this, SLOT(acceptError(QAbstractSocket::SocketError)));
    if (!fServer->listen(QHostAddress::Any, 888)) {
        QMessageBox::critical(nullptr, tr("Socket error"), tr("Cannot bind on port 888"));
    }
}

MTcpServer::~MTcpServer()
{
    qDebug() << "FSYO";
}

void MTcpServer::run()
{
    exec();
}

void MTcpServer::readyRead()
{
    auto *s = static_cast<QTcpSocket*>(sender());
    if (!fSockets.contains(s)) {
        return;
    }
    qint32 &fDataSize = fSocketDataSize[s];
    if (fDataSize == 0) {
        s->read(reinterpret_cast<char *>(&fDataSize), sizeof(qint32));
    }
    if (fDataSize <= 0) {
        quit();
        return;
    }
    QByteArray &fData = fSockets[s];
    fData.append(s->readAll());
    if (fData.length() >= fDataSize) {
        //QMutexLocker ml(&mxMap);
        MJsonHandler j(QHostAddress(), nullptr);
        fData = j.handle(fData);
        fDataSize = fData.length();
        s->write(reinterpret_cast<const char*>(&fDataSize), sizeof(qint32));
        s->write(fData.data(), fData.size());
        s->flush();
        s->waitForBytesWritten();
        s->close();
    }
}

void MTcpServer::disconnected()
{
    QTcpSocket *s = static_cast<QTcpSocket*>(sender());
    if (fSockets.contains(s)) {
        fSockets.remove(s);
        fSocketDataSize.remove(s);
    }
}

void MTcpServer::newConnection()
{
    if (fServer->hasPendingConnections()) {
        QTcpSocket *s = fServer->nextPendingConnection();
        fSocketDataSize[s] = 0;
        fSockets[s] = QByteArray();
        connect(s, SIGNAL(readyRead()), this, SLOT(readyRead()));
        connect(s, SIGNAL(disconnected()), this, SLOT(disconnected()));
        qApp->processEvents();
    }
}

void MTcpServer::acceptError(QAbstractSocket::SocketError err)
{
    qDebug() << err;
}
