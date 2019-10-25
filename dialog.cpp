#include "dialog.h"
#include "ui_dialog.h"
#include "mtcpserver.h"
#include "cnfapp.h"
#include "mjsonhandler.h"
#include "msqldatabase.h"
#include "dlgconnection.h"
#include "cnfmaindb.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QSettings>
#include <QProcess>
#include <windows.h>
#include <QFile>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    fStartDelay = 0;
    fExplorer = false;
    fCanClose = false;
    connect(&fTimer, SIGNAL(timeout()), this, SLOT(timeout()));
    fTimer.start(1000);
    ui->btnStore->setVisible(QFile::exists(qApp->applicationDirPath() + "/VeryFastF.exe"));
    ui->btnFastF->setVisible(QFile::exists(qApp->applicationDirPath() + "/FastF.exe"));
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::accept()
{
    if (fCanClose) {
        QDialog::accept();
    }
}

void Dialog::reject()
{
    if (fCanClose) {
        QDialog::reject();
    }
}

bool Dialog::checkPassword()
{
    if (!__cnfmaindb.fPassword.isEmpty()) {
        bool ok = false;
        QString pwd = QInputDialog::getText(this, tr("Password"), "", QLineEdit::Password, "", &ok);
        if (ok) {
            if (pwd != __cnfmaindb.fPassword) {
                QMessageBox::critical(this, tr("Error"), tr("Invalid password"));
                return false;
            }
        } else {
            return false;
        }
    }
    return true;
}

void Dialog::timeout()
{
#ifdef QT_DEBUG
    if (fStartDelay == 0) {
        init();
    }
    fStartDelay++;
    return;
#else
    fStartDelay++;
    if (fStartDelay == 10) {
        init();
    }
    if (!fExplorer) {
        QProcess::execute("taskkill /im explorer.exe /f");
        QProcess::execute("taskkill /im taskmgr.exe /f");
    }
#endif
}

void Dialog::on_btnConnection_clicked()
{
    if (!checkPassword()) {
        return;
    }
    auto *d = new DlgConnection(this);
    d->exec();
    delete d;
}

void Dialog::on_btnInstallShell_clicked()
{
#ifdef QT_DEBUG
    return;
#else
    QSettings s("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", QSettings::Registry64Format);
    s.setValue("Shell", "C:\\FastF\\FastFServer.exe");
#endif
}

void Dialog::on_btnRestoreShell_clicked()
{
    if (!checkPassword()) {
        return;
    }
    QSettings s("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", QSettings::Registry64Format);
    s.setValue("Shell", "Explorer.exe");
    QProcess p;
    p.startDetached("C:\\windows\\explorer.exe");
}

void Dialog::on_btnExplorer_clicked()
{
    if (!checkPassword()) {
        return;
    }
    fExplorer = true;
    QProcess p;
    p.startDetached("C:\\windows\\explorer.exe");
}

void Dialog::on_btnFastF_clicked()
{
    QProcess p;
    p.startDetached(qApp->applicationDirPath() + "/FastF.exe");
}

void Dialog::on_btnHideExplorer_clicked()
{
    fExplorer = false;
}


void Dialog::closeEvent(QCloseEvent *event)
{
    if (fCanClose) {
        QDialog::closeEvent(event);
        return;
    }
    event->ignore();
}
void Dialog::on_btnClose_clicked()
{
    if (!checkPassword()) {
        return;
    }
    fTimer.stop();
    fCanClose = true;
    accept();
    QProcess p;
    p.startDetached("C:\\windows\\explorer.exe");
}

void Dialog::on_btnRestart_clicked()
{
    fTimer.stop();
    fCanClose = true;
    QProcess p;
    p.startDetached("shutdown -f -t 1 -r");
}

void Dialog::on_btnShutDown_clicked()
{
    fTimer.stop();
    fCanClose = true;
    QProcess p;
    p.startDetached("shutdown -f -t 1 -s");
}

void Dialog::init()
{
    CnfApp::init(__cnfmaindb.fHost, __cnfmaindb.fDatabase, __cnfmaindb.fUser, __cnfmaindb.fPassword, "FASTF");
    MJsonHandler::fServerIp = __cnfmaindb.fServerIP;
    MSqlDatabase::setConnectionParams(__cnfmaindb.fHost, __cnfmaindb.fDatabase, __cnfmaindb.fUser, __cnfmaindb.fPassword);
    fServer = new MTcpServer();
    fServer->start();
    ui->lbTax->setText(QString("Fiscal: IP:%1, port %2, dept: %3")
                       .arg(__cnfapp.taxIP())
                       .arg(__cnfapp.taxPort())
                       .arg(__cnfapp.taxDept()));
}

void Dialog::on_btnStore_clicked()
{
    QProcess p;
    p.startDetached(qApp->applicationDirPath() + "/VeryFastF.exe");
}
