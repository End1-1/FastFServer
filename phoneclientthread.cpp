#include "phoneclientthread.h"
#include "phonejson.h"
#include <QByteArray>

PhoneClientThread::PhoneClientThread(QTcpSocket *s) :
    QThread()
{
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
    reset();
    mSocket = s;
    mHandlerCounter = 0;
    connect(mSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(mSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
}

PhoneClientThread::~PhoneClientThread()
{
    mSocket->close();
    mSocket->deleteLater();
}

void PhoneClientThread::run()
{
    exec();
}

void PhoneClientThread::reset()
{
    mDataSize = 0;
    mDataType = 0;
    mDataRead = 0;
}

void PhoneClientThread::readyRead()
{
    if (mDataSize == 0) {
        mSocket->read(reinterpret_cast<char *>(&mDataSize), sizeof(mDataSize));
    }
    if (mDataType == 0) {
        mSocket->read(reinterpret_cast<char *>(&mDataType), sizeof(mDataType));
    }
    mData.append(mSocket->read(mDataSize - mDataRead));
    mDataRead = mData.size();
    if (mDataRead == mDataSize) {
        mHandlerCounter++;
        switch (mDataType) {
        case 1: {
            auto *pj = new PhoneJson(mData, this);
            connect(pj, SIGNAL(reply(QByteArray)), this, SLOT(replyJSON(QByteArray)));
            pj->go();
            break;
        }
        }
        reset();
    }
}

void PhoneClientThread::disconnected()
{
    if (mHandlerCounter == 0) {
        quit();
    }
}

void PhoneClientThread::replyJSON(const QByteArray &data)
{
    mHandlerCounter--;
    quint32 dataSize = data.length();
    mSocket->write(reinterpret_cast<const char *>(&dataSize), sizeof(dataSize));
    mSocket->write(data);
    mSocket->flush();
    mSocket->waitForBytesWritten();
    sender()->deleteLater();
    if (mHandlerCounter == 0 && mSocket->state() != QTcpSocket::ConnectedState) {
        quit();
    }
}
