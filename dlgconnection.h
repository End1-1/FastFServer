#ifndef DLGCONNECTION_H
#define DLGCONNECTION_H

#include <QDialog>
#include "qsystem.h"

namespace Ui {
class DlgConnection;
}

class DlgConnection : public QDialog
{
    Q_OBJECT

public:
    explicit DlgConnection(QWidget *parent = nullptr);
    ~DlgConnection();
    QString path();
    QString user();
    QString pass();


private slots:
    void on_btnTest_clicked();
    void on_btnCancel_clicked();

public slots:
    void on_btnOk_clicked();

private:
    Ui::DlgConnection *ui;
    bool testConnection();
    QString m_lastDb;
    QString m_lastUser;
};

#endif // DLGCONNECTION_H
