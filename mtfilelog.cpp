#include "mtfilelog.h"
#include <QMutex>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QApplication>

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
    qDebug() << logFile << log;
    QStringList lst;
    lst << log;
    createLog(logFile, lst);
}

void MTFileLog::run()
{
    QMutexLocker m(&ml);
    QDir dir;
    if (!dir.exists(qApp->applicationDirPath() + "/logs")) {
        dir.mkdir(qApp->applicationDirPath() + "/logs");
    }
    QFile file(qApp->applicationDirPath() + "/logs/" + fLogFile + ".log");
    file.open(QIODevice::Append);
    QString begin = QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss --- ");
    file.write(begin.toUtf8());
    foreach (QString s, fLogList) {
        file.write(s.toUtf8());
        file.write("\r\n");
    }
    file.close();
}
