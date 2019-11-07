#ifndef PHONECLIENTTHREAD_H
#define PHONECLIENTTHREAD_H

#include <QThread>
#include <QTcpSocket>

class PhoneClientThread : public QThread
{
    Q_OBJECT
public:
    PhoneClientThread(QTcpSocket *s);
    ~PhoneClientThread();

protected:
    virtual void run() override;

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
