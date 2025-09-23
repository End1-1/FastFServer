#include "logwriter.h"
#include <QFile>
#include <QDateTime>
#include <QDebug>
#include <QDir>

QMutex LogWriter::fMutex;
int LogWriter::fCurrentLevel = 0;

LogWriter::LogWriter()
{

}

void LogWriter::write(const QString &file, const QString &message)
{
    QMutexLocker ml(&fMutex);
#ifdef QT_DEBUG
    qDebug() << QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss ") << message;
#endif;
    writeToFile(file, message);
    if (file != LogWriterLevel::verbose) {
        writeToFile(LogWriterLevel::verbose, message);
    }
}

void LogWriter::writeToFile(const QString &fileName, const QString &message)
{
    QString fnpath = QString("%1/%2/%3/Logs").arg(QDir::tempPath(), _APPLICATION_,_MODULE_);
    QString fn = fnpath + QString("/%4_%5.log").arg(QDate::currentDate().toString("dd_MM_yyyy"), fileName);
    QDir().mkpath(QDir().absoluteFilePath(fnpath));
    qDebug() << fn;
    QFile file(fn);
    if (file.open(QIODevice::Append)) {
        file.write(QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss ").toUtf8());
        file.write(message.toUtf8());
        file.write("\r\n");
        file.close();
    }
}
