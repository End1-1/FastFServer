#include "qsystem.h"
#include <QCoreApplication>
#include <QDir>
#include <QDate>

#ifdef WIN32
#include "windows.h"
#endif

QString QSystem::m_appPath;
QHostInfo QSystem::m_hostInfo;
QString QSystem::m_userId;
QString QSystem::m_userName;
QString QSystem::m_homePath;
QString QSystem::m_objectName;
bool QSystem::m_isMainDb;
int QSystem::m_dbGroup;
QString QSystem::m_dbPath;
bool QSystem::options_rep_show_complect_col;
bool QSystem::options_rep_show_prices_col;
bool QSystem::options_rep_gel_from_all_cafe;
QStringList QSystem::m_monthShort;

QString QSystem::WebUrl;
QString QSystem::WebPass;

QSystem::QSystem(char **argv, const QString &appName)
{
    m_appPath = argv[0];
    m_appPath = m_appPath.mid(0, m_appPath.lastIndexOf("\\"));
    QStringList paths = QCoreApplication::libraryPaths();
    paths.append(m_appPath);
    QCoreApplication::setLibraryPaths(paths);

    QDir dir;
    m_homePath = dir.homePath() + "/" + appName;
    if (!dir.exists(m_homePath))
        dir.mkdir(m_homePath);
    m_homePath += "/";

    m_monthShort.clear();
    m_monthShort << "jan" << "feb" << "mar" << "apr" << "may" << "jun" << "jul" << "aug" << "sep" << "oct" << "nov" << "dec";
    /*
#ifdef MANAGER_EXE
    if (!dir.exists(repDefaultPath()))
        if (!dir.mkpath(repDefaultPath()))
            return;
             TODO log("Cannot create default report directory: " + repDefaultPath(), false);
#endif
    */
}

QSystem::~QSystem()
{
}

QString QSystem::appPath()
{
    return m_appPath;
}

QString QSystem::homePath()
{
    return m_homePath;
}

QString QSystem::hostUserName()
{
    QString username = getenv("USER");
    if (username.isEmpty()) {
        username = getenv("USERNAME");
    }
    return username;
}

QString QSystem::hostInfo()
{
    return m_hostInfo.localHostName();
}

QString QSystem::userid()
{
    return m_userId;
}

QString QSystem::username()
{
    return m_userName;
}

QString QSystem::objectName()
{
    return m_objectName;
}

void QSystem::setUserData(const QString &username, const QString &userid)
{
    if (username.length())
        m_userName = username;
    if (userid.length())
        m_userId = userid;
}

void QSystem::setObjectName(const QString &objectName)
{
    m_objectName = objectName;
}

QString QSystem::dateTimeFormat()
{
    return DATETIME_FORMAT;
}

void QSystem::setMainFlag(bool flag)
{
    m_isMainDb = flag;
}

bool QSystem::isMain()
{
    return m_isMainDb;
}

void QSystem::setDbGroup(int id)
{
    m_dbGroup = id;
}

int QSystem::dbGroupId()
{
    return m_dbGroup;
}



QString QSystem::repDefaultPath()
{
    QDate d = QDate::currentDate();
    int year = d.year(), month = d.month() - 1;
    if (!month) {
        month = 11;
        year--;
    } else
        month--;
    QString path = QString("c:\\reports\\%1\\%2").arg(year).arg(m_monthShort.at(month));
    QDir dir;
    if (!dir.exists(path)) {
        dir.mkpath(path);
    }
    return path;
}

QString QSystem::shortMonth(QDate d)
{
    int month = d.month() - 1;
    return m_monthShort.at(month);
}

void QSystem::Log(const QString &msg)
{
    QFile f(homePath() + "/prg.log");
    if (f.open(QIODevice::Append)) {
        f.write(QDateTime::currentDateTime().toString(DATETIME_FORMAT).toLatin1());
        f.write(": ");
        f.write(msg.toLatin1());
        f.write("\r\n");
        f.close();
    }
}

QString getVersionString(QString fName)
{
// first of all, GetFileVersionInfoSize
    DWORD dwHandle;
    DWORD dwLen = GetFileVersionInfoSize(fName.toStdWString().c_str(), &dwHandle);

    // GetFileVersionInfo
    BYTE *lpData = new BYTE[dwLen];
    if(!GetFileVersionInfo(fName.toStdWString().c_str(), dwHandle, dwLen, lpData)) {
        delete [] lpData;
        return "";
    }

    // VerQueryValue
    VS_FIXEDFILEINFO *lpBuffer = NULL;
    UINT uLen;
    if(VerQueryValue(lpData, QString("\\").toStdWString().c_str(), (LPVOID*)&lpBuffer, &uLen)) {
        return
            QString::number((lpBuffer->dwFileVersionMS >> 16) & 0xffff) + "." +
            QString::number((lpBuffer->dwFileVersionMS) & 0xffff ) + "." +
            QString::number((lpBuffer->dwFileVersionLS >> 16 ) & 0xffff ) + "." +
            QString::number((lpBuffer->dwFileVersionLS) & 0xffff );
    }
    return "";
}

