#ifndef MTFILELOG_H
#define MTFILELOG_H

#include <QThread>

#define __LOG_SQL "sql"
#define __LOG_SQL_ERROR "sql.error"
#define __LOG_PRINT "print"
#define __LOG_OTHER "other"
#define __LOG_RECEIPT "recept"

class MTFileLog : public QThread
{
    Q_OBJECT
public:
    MTFileLog(const QString &logFile, const QStringList &logList);
    ~MTFileLog();
    static void createLog(const QString &logFile, const QStringList &logList);
    static void createLog(const QString &logFile, const QString &log);
protected:
    virtual void run();
private:
    QString fLogFile;
    QStringList fLogList;
};

#endif // MTFILELOG_H
