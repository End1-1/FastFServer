#ifndef QSYSTEM_H
#define QSYSTEM_H

#include <QObject>
#include <QHostInfo>
#include <QSettings>

#define DATE_FORMAT "dd.MM.yyyy"
#define DATETIME_FORMAT "dd.MM.yyyy HH:mm:ss"
#define TIME_FORMAT "HH:mm:ss"

#define DISH_STATE_NORMAL 1
#define DISH_STATE_REMOVED_NORMAL 2
#define DISH_STATE_REMOVED_PRINTED 3
#define DISH_STATE_MOVED 4
#define DISH_STATE_MOVE_PARTIALY 5
#define DISH_STATE_REMOVE_AFTER_CHECKOUT 6

#define ORDER_STATE_OPEN 1
#define ORDER_STATE_CLOSED 2
#define ORDER_STATE_EMTPY1 3 //Full empty
#define ORDER_STATE_EMPTY2 4 //Contain printed and then removed dishes
#define ORDER_STATE_REMOVED 5
#define ORDER_STATE_MERGE 6

#define LOG(v) QSystem::Log(v);

class QSystem
{
private:
    static QHostInfo m_hostInfo;
    static QString m_userId;
    static QString m_userName;
    static QString m_appPath;
    static QString m_homePath;
    static QString m_objectName;
    static bool m_isMainDb;
    static int m_dbGroup;
    static QString m_dbPath;
    static QStringList m_monthShort;

public:
    QSystem(char **argv, const QString &appName);
    ~QSystem();
    static QString appPath();
    static QString homePath();
    static QString hostUserName();
    static QString hostInfo();
    static QString userid();
    static QString username();
    static QString objectName();
    static void setUserData(const QString &username, const QString &userid);
    static void setObjectName(const QString &objectName);
    static QString dateTimeFormat();
    static void setMainFlag(bool flag);
    static bool isMain();
    static void setDbGroup(int id);
    static int dbGroupId();
    static QString repDefaultPath();
    static QString shortMonth(QDate d);
    static void Log(const QString &msg);

    static bool options_rep_show_prices_col;
    static bool options_rep_show_complect_col;
    static bool options_rep_gel_from_all_cafe;

    static QString WebUrl;
    static QString WebPass;
};

QString getVersionString(QString fName);


#endif // QSYSTEM_H
