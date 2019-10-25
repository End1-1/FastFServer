#include "mtprintkitchen.h"
#include "mtfilelog.h"
#include "ptextrect.h"
#include "pimage.h"
#include "cnfapp.h"
#include "msqldatabase.h"
#include "c5printing.h"
#include "QRCodeGenerator.h"
#include <QPrinterInfo>
#include <QPrinter>
#include <QPrintDialog>
#include <QDateTime>
#include <QPainter>
#include <QApplication>
#include <QDir>
#include <QMutexLocker>

static QMutex fMutex;

MTPrintKitchen::MTPrintKitchen(const QMap<QString, QString> &data, const QList<QMap<QString, QString> > &dishes, bool kitchen, QObject *parent) :
    QThread(parent)
{
    fDishes = dishes;
    fData = data;
    fKitchen = kitchen;
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

MTPrintKitchen::~MTPrintKitchen()
{
    qDebug() << "Print finished";
}

void MTPrintKitchen::run()
{
    fLog.append(QString("===================== Print ") + (fKitchen ? "Service" : "Receipt"));
    for (QMap<QString, QString>::const_iterator it = fData.begin(); it != fData.end(); it++) {
        fLog.append(it.key() + ": " + it.value());
    }
    for (int i = 0; i < fDishes.count(); i++) {
        QMap<QString, QString> &m = fDishes[i];
        QString temp;
        for (QMap<QString, QString>::const_iterator it = m.begin(); it != m.end(); it++) {
            temp += " " + it.key() + ": " + it.value();
        }
        fLog.append(temp);
    }
    fLog.append("Result ");
    if (fKitchen) {
        printKitchen();
    } else {
        printReceipt();
    }
    fLog.append("========================================================");
    //MTFileLog::createLog(__LOG_PRINT, fLog);
}

void MTPrintKitchen::printKitchen()
{
    if (fData["printer"].isEmpty()) {
        return;
    }

    QStringList printers = QPrinterInfo::availablePrinterNames();

    MSqlDatabase db;
    QMap<QString, QVariant> v;
    DatabaseResult dr;
    if (db.open()) {
        db.select("select map_value from sys_mobile_print_map where map_key='"
                  + fData["printer"] + "' and hall='" + fData["hall"] + "'", v, dr);
        if (dr.rowCount() > 0) {
            fLog.append("Print map: " + fData["printer"] + " ---> " + dr.value("MAP_VALUE").toString());
            fData["printer"] = dr.value("MAP_VALUE").toString();
        }
    }

    if (!printers.contains(fData["printer"], Qt::CaseInsensitive)) {
        fLog.append("Unavailable printer: " + fData["printer"]);
        return;
    }

    QFont font(qApp->font());
    font.setPointSize(20);
    C5Printing p;
    p.setSceneParams(650, 2700, QPrinter::Portrait);
    p.setFont(font);

    p.ctext(QString("%1 %2").arg(tr("Service check, order #")).arg(fData["order"]));
    p.br();
    p.ltext(QString("%1: %2").arg(tr("Table")).arg(fData["table"]), 0);
    p.br();
    p.ltext(QString("%1: %2").arg(tr("Date and time")).arg(QDateTime::currentDateTime().toString(def_date_time_format)), 0);
    p.br();
    p.ltext(QString("%1").arg(tr("Waiter")), 0);
    p.br();
    p.ltext(fData["staff"], 10);
    p.br();
    p.line();
    p.br(3);
    p.ltext(tr("Description"), 10);
    p.rtext(tr("Qty"));
    p.br();
    p.line();
    p.br(3);
    p.setFontBold(true);
    for (int i = 0; i < fDishes.count(); i++) {
        QMap<QString, QString> d = fDishes[i];
        p.ltext(d["dish"], 0);
        p.br();
        p.ltext(d["qty"], 0);
        p.br();
        if (!d["comments"].isEmpty()) {
            p.ltext(d["comments"], 0);
            p.br();
        }
        p.line(5);
        p.br(3);
    }
    p.setFontSize(16);
    p.ltext(tr("Printer: ") + fData["printer"], 0);
    p.br();
    p.ltext("_", 0);
    p.br();
    p.print(fData["printer"], QPrinter::Custom);
}

void MTPrintKitchen::printReceipt()
{
    QFont font(qApp->font());
    font.setPointSize(20);
    C5Printing p;
    p.setSceneParams(650, 2700, QPrinter::Portrait);
    p.setFont(font);

    QString imagePath = qApp->applicationDirPath() + "/logo_receipt.png";
    p.image(imagePath, Qt::AlignHCenter);
    p.br();

    /* Header */
    p.setFontBold(true);
    p.ctext(tr("Order number") + " " + fData["order"]);
    p.setFontBold(false);
    p.setFontSize(18);
    p.br();
    if (fData.contains("fiscal")) {
        p.ltext(fData["firm"], 0);
        p.br();
        p.ltext(fData["address"], 0);
        p.br();
        p.ltext(tr("Department"), 0);
        p.rtext(fData["dept"]);
        p.br();
        p.ltext(tr("GH"), 0);
        p.rtext(fData["gh"]);
        p.br();
        p.ltext(tr("Tax SN"), 0);
        p.rtext(fData["sn"]);
        p.br();
        p.ltext(tr("Fiscal"), 0);
        p.rtext(fData["fiscal"]);
        p.br();
        p.ltext(tr("Tax number"), 0);
        p.rtext(fData["taxnumber"]);
        p.br();
        p.ltext(tr("Tax payer"), 0);
        p.rtext(fData["hvhh"]);
        p.br();
        p.ltext(tr("(F)"), 0);
        p.br();
    }
    p.ltext(tr("Table"), 0);
    p.rtext(fData["table"]);
    p.br();
    p.ltext(tr("Date"), 0);
    p.rtext(fData["taxtime"]);
    p.br();
    p.ltext(tr("Staff"), 0);
    p.rtext(fData["staff"]);
    p.br();
    p.line(3);
    p.br(3);

    /* Dishes */
    double totalPre = 0.00;
    double total = 0;
    double totservice = 0;
    double totdiscount = 0;
    for (int i = 0; i < fDishes.count(); i++) {
        QMap<QString, QString> d = fDishes[i];
        p.ltext(tr("Class: ") + d["adgcode"] + ", " + d["dish"], 0);
        p.br();
        totalPre += d["qty"].toDouble() * d["price"].toDouble();
        QString name;
        if (d["price"].toDouble() < 1) {
            name = tr("Present");
        } else {
            name = d["qty"] + " x " + d["price"];
            double t1 = d["qty"].toDouble() * d["price"].toDouble(), t2 = 0;
            if (d["pm"].toInt() == 1) {
                if (fData["amount_inc_value"].toDouble() > 0.001) {
                    totservice += t1 * (fData["amount_inc_value"].toDouble() / 100);
                    t1 += t1 * (fData["amount_inc_value"].toDouble() / 100);
                }
                if (fData["amount_dec_value"].toDouble() > 0.001) {
                    t2 = t1 * (fData["amount_dec_value"].toDouble() / 100);
                    totdiscount += t2;
                }
            }
            name += " = " + float_str(d["qty"].toDouble() * d["price"].toDouble(), 2);
            total += t1 - t2;
        }
        p.ltext(name, 0);
        p.br();
        p.line();
        p.br(1);
    }
    p.br(3);
    p.line(3);
    p.setFontSize(20);
    p.ltext(tr("Counted"), 0);
    p.rtext(float_str(totalPre, 2));
    p.br();
    if (fData["amount_inc"].toDouble() > 0.1) {
        p.ltext(tr("Service included ") + fData["amount_inc_value"] + "%", 0);
        p.rtext(fData["amount_inc"]);
        p.br();
    }
    if (fData["amount_dec"].toDouble() > 0.1) {
        p.ltext(tr("Discount included ") + fData["amount_dec_value"] + "%", 0);
        p.rtext(fData["amount_dec"]);
        p.br();
    }
    p.setFontSize(22);
    p.setFontBold(true);
    p.ltext(tr("Grand total"), 0);
    p.rtext(fData["amount"]);
    p.br();
    p.setFontSize(18);
    p.setFontBold(false);
    p.ltext(tr("Thank You For Visit!"), 0);
    p.br();

    /*GIFT*/
//    if (fData.contains("gift_before")) {
//        top ++;
//        ps->addTextRect(10, top, rw, rh, tr("Gift balance before"), &rdl);
//        top += ps->addTextRect(10, top, rw, rh, fData["gift_before"], &rdr)->textHeight();
//        ps = checkTop(top, ps, lps);
//        ps->addTextRect(10, top, rw, rh, tr("Gift balance used"), &rdl);
//        top += ps->addTextRect(10, top, rw, rh, fData["gift_used"], &rdr)->textHeight();
//        ps = checkTop(top, ps, lps);
//        ps->addTextRect(10, top, rw, rh, tr("Gift balance after"), &rdl);
//        top += ps->addTextRect(10, top, rw, rh, fData["gift_after"], &rdr)->textHeight();
//        ps = checkTop(top, ps, lps);
//    }

    /* QRCode */
    p.setFontSize(20);
    p.br();
    p.br();
    p.br();
    p.ctext(tr("Payment avaiable with IDram"));
    p.br();

    int levelIndex = 1;
    int versionIndex = 0;
    bool bExtent = true;
    int maskIndex = -1;
    QString encodeString = QString("%1;%2;%3;%4|%5;%6;")
            .arg("Jazzve")
            .arg(__cnfapp.idramId()) //IDram ID
            .arg(fData["amount"])
            .arg(fData["order"])
            .arg(__cnfapp.idramPhone())
            .arg("1");

    CQR_Encode qrEncode;
    bool successfulEncoding = qrEncode.EncodeData( levelIndex, versionIndex, bExtent, maskIndex, encodeString.toUtf8().data() );
    if (!successfulEncoding) {
        fLog.append("Cannot encode qr image");
    }
    int qrImageSize = qrEncode.m_nSymbleSize;
    int encodeImageSize = qrImageSize + ( QR_MARGIN * 2 );
    QImage encodeImage( encodeImageSize, encodeImageSize, QImage::Format_Mono );
    encodeImage.fill( 1 );

    for ( int i = 0; i < qrImageSize; i++ ) {
        for ( int j = 0; j < qrImageSize; j++ ) {
            if ( qrEncode.m_byModuleData[i][j] ) {
                encodeImage.setPixel( i + QR_MARGIN, j + QR_MARGIN, 0 );
            }
        }
    }

    QPixmap pix = QPixmap::fromImage( encodeImage );
    pix = pix.scaled(300, 300);
    p.image(pix, Qt::AlignHCenter);
    p.br();
    /* End QRCode */

    p.setFontSize(16);
    p.ctext(QString("%1 %2").arg(tr("Printed:")).arg(QDateTime::currentDateTime().toString(def_date_time_format)));
    p.br();
    p.ctext("_");

    p.print(fData["printer"], QPrinter::Custom);
    fLog.append("Printer name: " + fData["printer"] + ", after: " + fData["printer"]);
    fLog.append("Printed");
}


PPrintScene *MTPrintKitchen::checkTop(int &top, PPrintScene *ps, QList<PPrintScene *> &lps)
{
    if (top > sizePortrait.height() - 200) {
        top = 10;
        ps = new PPrintScene(Portrait, nullptr);
        lps.append(ps);
    }
    return ps;
}
