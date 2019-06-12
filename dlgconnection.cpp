#include "dlgconnection.h"
#include "ui_dlgconnection.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QMessageBox>
#include <QFile>
#include "qsystem.h"
#include "cnfmaindb.h"

#define buff_size 255 * 7

DlgConnection::DlgConnection(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgConnection)
{
    ui->setupUi(this);
    ui->lnServerIP->setText(__cnfmaindb.fServerIP);
    ui->lnHost->setText(__cnfmaindb.fHost);
    ui->lnPath->setText(__cnfmaindb.fDatabase);
    ui->lnUser->setText(__cnfmaindb.fUser);
    ui->lnPass->setText(__cnfmaindb.fPassword);
    ui->chServerMode->setChecked(__cnfmaindb.fServerMode.toInt() > 0);
    ui->leTaxDepartment->setText(__cnfmaindb.fTaxDepartment);
}

DlgConnection::~DlgConnection()
{
    delete ui;
}

QString DlgConnection::path()
{
    return ui->lnPath->text();
}

QString DlgConnection::user()
{
    return ui->lnUser->text();
}

QString DlgConnection::pass()
{
    return ui->lnPass->text();
}


void DlgConnection::on_btnTest_clicked()
{
    if (testConnection())
        QMessageBox::information(this, tr("Connection test"), tr("Connected"));
}

void DlgConnection::on_btnCancel_clicked()
{
    reject();
}

void DlgConnection::on_btnOk_clicked()
{
    CnfMainDb::fServerIP = ui->lnServerIP->text();
    CnfMainDb::fHost = ui->lnHost->text();
    CnfMainDb::fDatabase = ui->lnPath->text();
    CnfMainDb::fUser = ui->lnUser->text();
    CnfMainDb::fPassword = ui->lnPass->text();
    CnfMainDb::fServerMode = ui->chServerMode->isChecked() ? "1" : "0";
    CnfMainDb::fTaxDepartment = ui->leTaxDepartment->text();
    CnfMainDb::write();
    accept();
}

bool DlgConnection::testConnection()
{
    bool result;
    QSqlDatabase db = QSqlDatabase::addDatabase("QIBASE", "test");
    db.setHostName(ui->lnHost->text());
    db.setDatabaseName(ui->lnPath->text());
    db.setUserName(ui->lnUser->text());
    db.setPassword(ui->lnPass->text());
    result = db.open();
    if (!result)
        QMessageBox::critical(this, tr("Connection test"), tr("Connection error") + "\r\n" + db.lastError().databaseText());
    QSqlDatabase::removeDatabase("test");
    return result;
}
