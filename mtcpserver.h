#ifndef MTCPSERVER_H
#define MTCPSERVER_H

#include <QThread>
#include <QTcpServer>

class MTcpServer : public QObject
{
    Q_OBJECT

public:
    MTcpServer(QObject *parent = nullptr);
    ~MTcpServer();

public slots:
    void run();

private:
    QMap<QTcpSocket*, qint32> fSocketDataSize;
    QMap<QTcpSocket*, QByteArray> fSockets;
    QTcpServer *fServer;

private slots:
    void readyRead();
    void disconnected();
    void newConnection();
    void acceptError(QAbstractSocket::SocketError err);
};

#endif // MTCPSERVER_H
