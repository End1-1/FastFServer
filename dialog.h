#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTimer>

namespace Ui {
class Dialog;
}

class MTcpServer;

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog() override;
    virtual void accept() override;
    virtual void reject() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    bool checkPassword();

private slots:
    void timeout();
    void on_btnConnection_clicked();
    void on_btnInstallShell_clicked();
    void on_btnRestoreShell_clicked();
    void on_btnExplorer_clicked();
    void on_btnFastF_clicked();
    void on_btnHideExplorer_clicked();
    void on_btnClose_clicked();
    void on_btnRestart_clicked();
    void on_btnShutDown_clicked();
    void on_btnStore_clicked();

private:
    Ui::Dialog *ui;
    MTcpServer *fServer;
    QTimer fTimer;
    bool fExplorer;
    bool fCanClose;
    int fStartDelay;
    void init();
};

#endif // DIALOG_H
