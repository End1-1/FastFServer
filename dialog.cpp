#include "dialog.h"
#include "ui_dialog.h"
#include "dlgconnection.h"
#include "cnfmaindb.h"
#include "logwriter.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QSettings>
#include <QProcess>
#include <windows.h>
#include <QFile>

Dialog* __logDialog = nullptr;

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
    ui->btnSalary->setVisible(QFile::exists(qApp->applicationDirPath() + "/Cafe.exe"));
    __logDialog = this;
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::accept()
{
    if(fCanClose) {
        QDialog::accept();
    }
}

void Dialog::reject()
{
    if(fCanClose) {
        QDialog::reject();
    }
}

bool Dialog::checkPassword()
{
    if(!__cnfmaindb.fPassword.isEmpty()) {
        bool ok = false;
        QString pwd = QInputDialog::getText(this, tr("Password"), "", QLineEdit::Password, "", &ok);

        if(ok) {
            if(pwd != __cnfmaindb.fPassword) {
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
    fStartDelay++;
    return;
#else
    fStartDelay++;

    if(!fExplorer) {
        QProcess::execute("taskkill /im explorer.exe /f");
        QProcess::execute("taskkill /im taskmgr.exe /f");
    }

#endif
}

void Dialog::logMsg(const QString &msg)
{
    qDebug() << msg;
    LogWriter::write(LogWriterLevel::verbose, msg);
}

void Dialog::on_btnConnection_clicked()
{
    if(!checkPassword()) {
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
    if(!checkPassword()) {
        return;
    }

    QSettings s("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", QSettings::Registry64Format);
    s.setValue("Shell", "Explorer.exe");
    QProcess p;
    p.startDetached("C:\\windows\\explorer.exe");
}

void Dialog::on_btnExplorer_clicked()
{
    if(!checkPassword()) {
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
    if(fCanClose) {
        QDialog::closeEvent(event);
        return;
    }

    event->ignore();
}
void Dialog::on_btnClose_clicked()
{
    if(!checkPassword()) {
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

void Dialog::on_btnStore_clicked()
{
    QProcess p;
    p.startDetached(qApp->applicationDirPath() + "/VeryFastF.exe");
}

void Dialog::on_btnSalary_clicked()
{
    QProcess p;
    p.startDetached(qApp->applicationDirPath() + "/Cafe.exe");
}
