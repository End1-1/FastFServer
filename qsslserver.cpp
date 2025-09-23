
#include "qsslserver.h"
#include <QSslSocket>
#include <QFile>

QSslServer::QSslServer(QObject *parent) :
    QTcpServer(parent)
{

}

void QSslServer::setSslLocalCertificate(const QSslCertificate &s)
{
    fSslLocalCertificate = s;
}

bool QSslServer::setSslLocalCertificate(const QString &path, QSsl::EncodingFormat format)
{
    QFile certificateFile(path);
    if (!certificateFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    fSslLocalCertificate = QSslCertificate(certificateFile.readAll(), format);
    return true;
}

void QSslServer::setSslPrivateKey(const QSslKey &k)
{
    fSslPrivateKey = k;
}

bool QSslServer::setSslPrivateKey(const QString &path, QSsl::KeyAlgorithm algorithm, QSsl::EncodingFormat format, const QByteArray &passPhrase)
{
    QFile keyFile(path);
    if (!keyFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    fSslPrivateKey = QSslKey(keyFile.readAll(), algorithm, format, QSsl::PrivateKey, passPhrase);
    return true;
}

void QSslServer::setSslProtocol(QSsl::SslProtocol p)
{
    fSslProtocol = p;
}

void QSslServer::incomingConnection(qintptr handle)
{
//    QTcpServer::incomingConnection(handle);
    QSslSocket *sslSocket = new QSslSocket(this);
    sslSocket->setSocketDescriptor(handle);
    sslSocket->setLocalCertificate(fSslLocalCertificate);
    sslSocket->setPrivateKey(fSslPrivateKey);
    sslSocket->setProtocol(fSslProtocol);
    sslSocket->startServerEncryption();
    this->addPendingConnection(sslSocket);
}
