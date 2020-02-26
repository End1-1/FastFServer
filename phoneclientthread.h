#ifndef PHONECLIENTTHREAD_H
#define PHONECLIENTTHREAD_H

#include "threadobject.h"
#include <QTcpSocket>

class PhoneClientThread : public QObject
{
    Q_OBJECT
public:
    PhoneClientThread(QTcpSocket *s);
    ~PhoneClientThread();

private:
    int mHandlerCounter;
    quint32 mDataSize;
    quint32 mDataRead;
    quint32 mDataType;
    QByteArray mData;
    QTcpSocket *mSocket;
    void reset();

private slots:
    void readyRead();
    void disconnected();
    void replyJSON(const QByteArray &data);
};

#endif // PHONECLIENTTHREAD_H
