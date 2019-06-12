#include "mtfilelog.h"
#include <QMutex>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QDebug>

static QMutex ml;

MTFileLog::MTFileLog(const QString &logFile, const QStringList &logList) :
    QThread(nullptr)
{
    fLogFile = logFile;
    fLogList = logList;
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

MTFileLog::~MTFileLog()
{
    qDebug() << "MTFileLog destructor";
}

void MTFileLog::createLog(const QString &logFile, const QStringList &logList)
{
    MTFileLog *lf = new MTFileLog(logFile, logList);
    lf->start();
}

void MTFileLog::createLog(const QString &logFile, const QString &log)
{
    QStringList lst;
    lst << log;
    createLog(logFile, lst);
}

void MTFileLog::run()
{
    QMutexLocker m(&ml);
    QDir dir;
    if (!dir.exists("logs")) {
        dir.mkdir("logs");
    }
    QFile file("logs/" + fLogFile + ".log");
    file.open(QIODevice::Append);
    QString begin = QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss --- ");
    file.write(begin.toUtf8());
    foreach (QString s, fLogList) {
        file.write(s.toUtf8());
        file.write("\r\n");
    }
    file.close();
}
