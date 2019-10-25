#include "mjsonhandler.h"
#include "msqldatabase.h"
#include "cnfmaindb.h"
#include "mdefined.h"
#include "mtprintkitchen.h"
#include "mtfilelog.h"
#include "cnfapp.h"
#include "printtaxn.h"
#include <QUuid>
#include <QNetworkInterface>
#include <QCryptographicHash>
#include <QPrinterInfo>
#include <QFile>
#include <QMutex>

QString MJsonHandler::fServerIp;
QMap<QString, int> MJsonHandler::fSessions;
QMap<int, QString> MJsonHandler::fLockTables;

MJsonHandler::MJsonHandler(const QHostAddress &clientAddr, QObject *parent) : QObject(parent)
{

    fClientAddr = QHostAddress(clientAddr.toIPv4Address());
    if (!fDb.configure()) {
        fError = fDb.lastError();
    }
}

QByteArray MJsonHandler::handle(const QByteArray &rawData)
{
    QJsonDocument jDoc = QJsonDocument::fromJson(rawData);
    QJsonObject jObj = jDoc.object();
    if (jObj.contains("server")) {
        return handleServer(jObj);
    } else if (jObj.contains("query")) {
        return handleQuery(jObj);
    }
    jObj["error"] = "Unknown query";
    jDoc = QJsonDocument(jObj);
    fDb.commit();
    return jDoc.toJson();
}

QByteArray MJsonHandler::handleDish(const QJsonObject &o)
{
    Q_UNUSED(o)
    DatabaseResult dr;
    QMap<QString, QVariant> bind;
    bind[":menuid"] = o["menuid"].toString().toInt();
    fDb.select("select md.id, md.type_id, md.name, mm.price, mm.print1, mm.print2, mm.store_id, md.remind "
              "from me_dishes_menu mm "
              "inner join me_dishes md on md.id=mm.dish_id "
              "where mm.menu_id=:menuid and mm.state_id=1 ", bind, dr);
    QJsonArray jArr;
    for (int i = 0; i < dr.rowCount(); i++) {
        QJsonObject jt;
        jt["id"] = dr.toString(i, "ID");
        jt["type"] = dr.toString(i, "TYPE_ID");
        jt["name"] = dr.toString(i, "NAME");
        jt["price"] = dr.toString(i, "PRICE");
        jt["print1"] = dr.toString(i, "PRINT1");
        jt["print2"] = dr.toString(i, "PRINT2");
        jt["store"] = dr.toString(i, "STORE_ID");
        jt["remind"] = dr.toString(i, "REMIND");
        jArr.push_back(jt);
    }
    QJsonObject jObj;
    jObj["reply"] = "ok";
    jObj["list"] = jArr;
    jObj["type"] = "dish";
    QJsonDocument jDoc(jObj);
    return jDoc.toJson();
}

QByteArray MJsonHandler::handleDishType(const QJsonObject &o)
{
    Q_UNUSED(o)
    DatabaseResult dr;
    QMap<QString, QVariant> bind;
    bind[":menuid"] = o["menuid"].toString().toInt();
    fDb.select("select id, part_id, name from me_types where id in "
              "(select type_id from me_dishes where id in "
              "(select dish_id from me_dishes_menu where menu_id=:menuid and state_id=1))", bind, dr);
    QJsonArray jArr;
    for (int i = 0; i < dr.rowCount(); i++) {
        QJsonObject jt;
        jt["id"] = dr.value(i, "ID").toString();
        jt["part"] = dr.value(i, "PART_ID").toString();
        jt["name"] = dr.value(i, "NAME").toString();
        jArr.push_back(jt);
    }
    QJsonObject jObj;
    jObj["reply"] = "ok";
    jObj["list"] = jArr;
    jObj["type"] = "dishtype";
    QJsonDocument jDoc(jObj);
    return jDoc.toJson();
}

QByteArray MJsonHandler::handleHall(const QJsonObject &o)
{
    Q_UNUSED(o)
    MSqlDatabase db;

    DatabaseResult dr;
    QMap<QString, QVariant> bind;
    fDb.select("select t.id, t.hall_id, t.name, o.staff_id, e.fname || ' ' || e.lname as staff_name, "
              "o.amount, o.print_qty "
              "from h_table t "
              "left join o_order o on o.id=t.order_id "
              "left join employes e on e.id=o.staff_id "
              "order by queue", bind, dr);
    QJsonArray jArr;
    for (int i = 0; i < dr.rowCount(); i++) {
        QJsonObject jt;
        jt["id"] = dr.value(i, "ID").toString();
        jt["hall"] = dr.value(i, "HALL_ID").toString();
        jt["name"] = dr.value(i, "NAME").toString();
        jt["staffid"] = dr.value(i, "STAFF_ID").toString();
        jt["staffname"] = dr.value(i, "STAFF_NAME").toString();
        jt["amount"] = float_str(dr.value(i, "AMOUNT").toDouble(), 0);
        jt["printed"] = dr.value(i, "PRINT_QTY").toString();
        jArr.push_back(jt);
    }
    QJsonObject jObj;
    jObj["reply"] = "ok";
    jObj["list"] = jArr;
    jObj["type"] = "hall";
    QJsonDocument jDoc(jObj);
    return jDoc.toJson();
}

QByteArray MJsonHandler::handleQuery(const QJsonObject &o)
{
    if (!fError.isEmpty()) {
        return jsonError(fError);
    }
    if (!o.contains("session")) {
        return jsonError(tr("No session id"));
    }
    if (!fSessions.contains(o["session"].toString()) && o["session"] != "session_admin_id") {
        return jsonError(tr("Invalid session id"));
    }
    QJsonObject jObj;
    if (o["query"] == "hall") {
        return handleHall(o);
    } else if (o["query"] == "dishtype") {
        return handleDishType(o);
    } else if (o["query"] == "dish") {
        return handleDish(o);
    } else if (o["query"] == "locktable") {
        return handleLockTable(o);
    } else if (o["query"] == "unlocktable") {
        return handleUnlockTable(o);
    } else if (o["query"] == "dishtoorder") {
        return handleDishToOrder(o);
    } else if (o["query"] == "opentable") {
        return handleOpenTable(o);
    } else if (o["query"] == "loadorder") {
        return handleLoadOrder(o);
    } else if (o["query"] == "dishchange") {
        return handleDishChange(o);
    } else if (o["query"] == "removedish") {
        return handleDishRemove(o);
    } else if (o["query"] == "printkitchen") {
        return handlePrintKitchen(o);
    } else if (o["query"] == "printreceipt") {
        return handleReceipt(o);
    } else if (o["query"] == "listofsettings") {
        return handleSettings(o);
    } else if (o["query"] == "listreceipt") {
        return handleListReceipt(o);
    } else if(o["query"] == "closeorder") {
        return handleCloseOrder(o);
    } else if (o["query"] == "odcomment") {
        return handleDishComment(o);
    } else if (o["query"] == "commentslist") {
        return handleCommentsList(o);
    } else if (o["query"] == "readydishes") {
        return handleReadyDishes(o);
    } else if (o["query"] == "readydishdone") {
        return handleReadyDishDone(o);
    } else if (o["query"] == "mymoney") {
        return handleMyMoney(o);
    } else if (o["query"] == "getreminders") {
        return handleGetReminders(o);
    } else if (o["query"] == "updatereminder") {
        return handleReminder(o);
    }
    jObj["reply"] = tr("Invalid query");
    QJsonDocument jDoc(jObj);
    return jDoc.toJson();
}

QByteArray MJsonHandler::handleServer(const QJsonObject &o)
{
    if (!fError.isEmpty()) {
        return jsonError(fError);
    }
    QMap<QString, QVariant> v;
    DatabaseResult dr;
    QJsonObject jObj;
    if (o["server"].toString() == "who") {
        if (o.contains("auth")) {
            QString pwd = o["auth"].toString();
            if (int user = checkPassword(pwd)) {
                foreach (QString s, fSessions.keys(user)) {
                    fSessions.remove(s);
                }
                v[":id"] = user;
                fDb.select("select e.fname || ' ' || e.lname as staffname from employes e where id=:id", v, dr);

                jObj["server"] = "ok";
                jObj["session"] = sessionNew();
                fSessions[jObj["session"].toString()] = user;
                jObj["ip"] = fServerIp;
                jObj["staffid"] = QString::number(user);
                jObj["staffname"] = dr.value("STAFFNAME").toString();
            } else {
                jObj["server"] = "no";
            }
        } else if (o.contains("session")) {
            QString session = o["session"].toString();
            if (fSessions.contains(session)) {
                jObj["server"] = "ok";
                jObj["session"] = session;
                jObj["ip"] = fServerIp;
                jObj["warm"] = "no";
                jObj["staffid"] = QString::number(fSessions[session]);
                v[":id"] = fSessions[session];
                fDb.select("select e.fname || ' ' || e.lname as staffname from employes e where id=:id", v, dr);
                jObj["staffname"] = dr.value("STAFFNAME").toString();
            }
        } else {
            jObj["server"] = "no";
        }
    }
    QJsonDocument jDoc(jObj);
    return jDoc.toJson();
}

QByteArray MJsonHandler::handleLockTable(const QJsonObject &o)
{
    QMap<QString, QVariant> v;
    DatabaseResult dr;
    v[":id"] = o["table"].toString().toInt();
    fDb.select("select lock_host from h_table where id=:id", v, dr);
    bool ok = true;
    if (dr.rowCount() > 0) {
        QString lockHost = dr.value("LOCK_HOST").toString();

        int table = o["table"].toString().toInt();
        if (fLockTables.contains(table)) {
            if (fLockTables[o["table"].toString().toInt()] != o["session"].toString()) {
                if (fSessions.contains(fLockTables[o["table"].toString().toInt()])) {
                    ok = false;
                }
            }
            if (!lockHost.isEmpty()) {
                if (lockHost != o["session"].toString()) {
                    int user = fSessions[o["session"].toString()];
                    int oldUser = fSessions[lockHost];
                    if (user != oldUser) {
                        if (!lockHost.contains("{")) {
                            ok = false;
                        }
                    }
                }
            }
        }

        if (ok) {
            fLockTables[table] = o["session"].toString();
            v[":id"] = o["table"].toString().toInt();
            v[":lock_host"] = o["session"].toString();
            fDb.select("update h_table set lock_host=:lock_host where id=:id", v, dr);
        }
    } else {
        ok = false;
    }
    QJsonObject obj;
    obj["reply"] = (ok ? "ok" : tr("Table locked"));
    obj["locktable"] = o["table"];
    QJsonDocument jDoc(obj);
    return jDoc.toJson();
}

QByteArray MJsonHandler::handleUnlockTable(const QJsonObject &o)
{
    QMap<QString, QVariant> v;
    DatabaseResult dr;
    v[":id"] = o["table"].toString().toInt();
    fDb.select("select order_id from h_table where id=:id", v, dr);
    QString order = dr.value("ORDER_ID").toString();
    if (!order.isEmpty()) {
        v[":state_id"] = DISH_STATE_NORMAL;
        v[":order_id"] = order;
        fDb.select("select count(id) as id from o_dishes where state_id=:state_id and order_id=:order_id", v, dr);
        if (dr.value("ID").toInt() == 0) {
            v[":state_id"] = ORDER_STATE_EMTPY1;
            v[":order_id"] = order;
            fDb.select("update o_order set state_id=:state_id where id=:id", v, dr);
            v[":id"] = o["table"].toString().toInt();
            fDb.select("update h_table set order_id='' where id=:id", v, dr);
        }
    }
    v[":lock_host"] = o["session"].toString();    
    fDb.select("update h_table set lock_host='' where lock_host=:lock_host", v, dr);
    fLockTables.remove(o["table"].toString().toInt());
    QJsonObject jReply;
    jReply["reply"] = "ok";
    return jsonreply(jReply);
}

QByteArray MJsonHandler::handleDishToOrder(const QJsonObject &o)
{
    int table = o["table"].toString().toInt();
    QString order = o["order"].toString();
    QMap<QString, QVariant> v;
    if (order.isEmpty()) {
        DatabaseResult dorder;
        v[":table_id"] = table;
        v[":state_id"] = 1;
        if(!fDb.select("select id from o_order where table_id=:table_id and state_id=:state_id", v, dorder, false)) {
            return jsonError(fDb.lastError());
        }
        if (dorder.rowCount() == 0) {
            order = QString("%1-%2").arg("J3").arg(fDb.genId("gen_o_order_id"));
            v[":order_id"] = order;
            if (!fDb.update("h_table", v, table)) {
                return jsonError(fDb.lastError());
            }
            v[":id"] = order;
            v[":state_id"] = 1;
            v[":table_id"] = table;
            v[":date_open"] = QDateTime::currentDateTime();
            v[":date_close"] = QDateTime::currentDateTime();
            v[":date_cash"] = QDate::currentDate();
            v[":staff_id"] = fSessions[o["session"].toString()];
            v[":print_qty"] = 0;
            v[":amount_inc"] = 0;
            v[":amount_dec"] = 0;
            v[":amount_inc_value"] = o["priceinc"].toString().toDouble();
            v[":amount_dec_value"] = 0;
            v[":payment"] = 1;
            v[":taxprint"] = 0;
            v[":comment"] = "";
            v[":cash_id"] = 3;
            if (!fDb.insert("o_order", v)) {
                return fDb.lastError().toUtf8();
            }
        } else {
            order = dorder.value("ID").toString();
        }
    }
    v[":id"] = order;
    DatabaseResult dr;
    fDb.select("select print_qty from o_order where id=:id", v, dr, false);
    if (dr.rowCount() > 0) {
        if (dr.value("PRINT_QTY").toInt() > 0) {
            return jsonError(tr("Only close order is available"));
        }
    }

    int rec = fDb.genId("gen_o_dish_id");
    v[":id"] = rec;
    v[":order_id"] = order;
    v[":state_id"] = 1;
    v[":dish_id"] = o["dish"].toString().toInt();
    v[":qty"] = o["qty"].toString().toDouble();
    v[":printed_qty"] = 0;
    v[":price"] = o["price"].toString().toDouble();
    v[":price_inc"] = o["priceinc"].toString().toDouble();
    v[":price_dec"] = 0;
    v[":last_user"] = fSessions[o["session"].toString()];
    v[":store_id"] = o["store"].toString().toInt();
    v[":payment_mod"] = 1;
    v[":comments"] = "";
    v[":remind"] = o["remind"].toString().toInt();
    v[":print1"] = o["print1"].toString();
    v[":print2"] = o["print2"].toString();
    fDb.insert("o_dishes", v);

    QString total = updateOrderAmount(order);

    QJsonObject jObj;
    jObj["order"] = order;
    jObj["rec"] = rec;
    jObj["reply"] = "ok";
    jObj["total"] = total;
    return QJsonDocument(jObj).toJson();
}

QByteArray MJsonHandler::handleOpenTable(const QJsonObject &o)
{
    int table = o["table"].toString().toInt();
    QMap<QString, QVariant> v;
    v[":table_id"] = table;
    v[":state_id"] = 1;
    DatabaseResult dr;
    fDb.select("select id, amount from o_order where table_id=:table_id and state_id=:state_id", v, dr);
    QJsonObject obj;
    if (dr.rowCount() > 0) {
        obj["reply"] = "ok";
        obj["order"] = dr.value("ORDER_ID").toString();
        obj["amount"] = float_str(dr.value("AMOUNT").toDouble(), 0);
        //db.select("",  v, dr); //TODO: I dont know why
    } else {
        obj["reply"] = "ok";
        obj["order"] = "";
    }
    return QJsonDocument(obj).toJson();
}

QByteArray MJsonHandler::handleLoadOrder(const QJsonObject &o)
{
    QString order = o["order"].toString();
    if (order.isEmpty()) {
        return jsonError("Incorrect order number");
    }

    QJsonObject obj;
    QMap<QString, QVariant> v;
    DatabaseResult dr;
    v[":id"] = order;
    fDb.select("select o.state_id, e.fname || ' ' || e.lname as staff, o.amount from o_order o "
              "left join employes e on e.id=o.staff_id "
              "where o.id=:id ", v, dr);
    if (dr.rowCount() == 0) {
        return jsonError("Incorrect order number");
    }
    if (dr.toInt(0, "STATE_ID") != 1) {
        return jsonError("Order state is not opened");
    }
    obj["reply"] = "ok";
    obj["staff"] = dr.toString(0, "STAFF");
    obj["amount"] = float_str(dr.value(0, "AMOUNT").toDouble(), 0);
    v[":order_id"] = order;
    fDb.select("select od.id, od.dish_id, md.name, od.qty, od.printed_qty, od.comments, "
              "od.print1, od.print2, od.price "
              "from  o_dishes od "
              "left join me_dishes md on md.id=od.dish_id "
              "where od.order_id=:order_id and od.state_id=1 ", v, dr);
    QJsonArray ja;
    for (int i = 0; i < dr.rowCount(); i++) {
        QJsonObject od;
        od["recid"] = dr.toString(i, "ID");
        od["dishid"] = dr.toString(i, "DISH_ID");
        od["dishname"] = dr.toString(i, "NAME");
        od["qty"] = dr.toString(i, "QTY");
        od["printedqty"] = dr.toString(i, "PRINTED_QTY");
        od["comments"] = dr.toString(i, "COMMENTS");
        od["price"] = dr.toString(i, "PRICE");
        od["print1"] = dr.toString(i, "PRINT1");
        od["print2"] = dr.toString(i, "PRINT2");
        ja.append(od);
    }
    obj["dishes"] = ja;
    return jsonreply(obj);
}

QByteArray MJsonHandler::handleDishChange(const QJsonObject &o)
{
    QMap<QString, QVariant> v;
    v[":id"] = o["recid"].toString().toInt();
    DatabaseResult dr;
    fDb.select("select order_id from o_dishes where id=:id", v, dr, false);
    QString order;
    if (dr.rowCount() > 0) {
        order = dr.value("ORDER_ID").toString();
    }
    v[":id"] = order;
    fDb.select("select print_qty from o_order where id=:id", v, dr, false);
    if (dr.rowCount() > 0) {
        if (dr.value("PRINT_QTY").toInt() > 0) {
            return jsonError(tr("Only close order is available"))       ;
        }
    }

    v[":qty"] = o["qty"].toString().toDouble();
    v[":comments"] = o["comments"].toString();
    if (!fDb.update("o_dishes", v, o["recid"].toString().toInt())) {
        return jsonError(fDb.lastError());
    }

    v[":id"] = o["recid"].toString().toInt();
    fDb.select("select order_id from o_dishes where id=:id", v, dr);
    QString total = "0";
    if (dr.rowCount() > 0) {
        total = updateOrderAmount(dr.value("ORDER_ID").toString());
    }
    QJsonObject obj;
    obj["reply"] = "ok";
    obj["total"] = total;
    return jsonreply(obj);
}

QByteArray MJsonHandler::handleDishRemove(const QJsonObject &o)
{
    QMap<QString, QVariant> v;
    v[":state_id"] = DISH_STATE_REMOVED_NORMAL;
    if (!fDb.update("o_dishes", v, o["recid"].toString().toInt())) {
        return jsonError(fDb.lastError());
    }
    DatabaseResult dr;
    v[":id"] = o["recid"].toString().toInt();
    fDb.select("select order_id from o_dishes where id=:id", v, dr);
    QString total = "0";
    if (dr.rowCount() > 0) {
        total = updateOrderAmount(dr.value("ORDER_ID").toString());
    }
    QJsonObject obj;
    obj["reply"] = "ok";
    obj["total"] = total;
    return jsonreply(obj);
}

QByteArray MJsonHandler::handlePrintKitchen(const QJsonObject &o)
{
    QString order = o["order"].toString();
    if (order.isEmpty()) {
        return jsonError("Incorrect order number");
    }
    QMap<QString, QVariant> v;
    DatabaseResult dr;
    v[":order_id"] = order;
    v[":state_id"] = DISH_STATE_NORMAL;
    fDb.select("select od.id, md.name, od.qty-od.printed_qty as qty, od.comments, od.print1, od.print2, "
              "od.remind, od.dish_id "
              "from o_dishes od "
              "left join me_dishes md on md.id=od.dish_id "
              "where od.order_id=:order_id and od.state_id=:state_id and od.printed_qty<od.qty "
              "and (char_length(od.print1)>0 or char_length(od.print2)>0) ", v, dr);
    DatabaseResult dh;
    v[":id"] = order;
    fDb.select("select h.name as hname, t.name tname, e.fname || ' ' || e.lname ename, "
              "o.staff_id, o.table_id "
              "from o_order o "
              "left join h_table t on t.id=o.table_id "
              "left join h_hall h on h.id=t.hall_id "
              "left join employes e on e.id=o.staff_id "
              "where o.id=:id", v, dh);
    QMap<QString, QString> data;
    data["order"] = order;
    data["hall"] = dh.toString(0, "HNAME");
    data["table"] = dh.toString(0, "TNAME");
    data["staff"] = dh.toString(0, "ENAME");
    QSet<QString> prn1;
    QSet<QString> prn2;
    for (int i = 0; i < dr.rowCount(); i++) {
        prn1.insert(dr.value(i, "PRINT1").toString());
        if (dr.value(i, "PRINT1").toString() != dr.value(i, "PRINT2").toString()) {
            prn2.insert(dr.value(i, "PRINT2").toString());
        }
    }
    foreach (QString prn, prn1) {
        data["printer"] = prn;
        QList<QMap<QString, QString> > dishes;
        for (int i = 0; i < dr.rowCount(); i++) {
            if (dr.toString(i, "PRINT1") == prn) {
                QMap<QString, QString> d;
                d["qty"] = float_str(dr.toDouble(i, "QTY"), 1);
                d["dish"] = dr.toString(i, "NAME");
                d["comments"] = dr.toString(i, "COMMENTS");
                dishes.append(d);
            }
        }
        if (dishes.count() > 0) {
            MTPrintKitchen *p = new MTPrintKitchen(data, dishes, true);
            p->start();
        }

    }
    foreach (QString prn, prn2) {
        data["printer"] = prn;
        QList<QMap<QString, QString> > dishes;
        for (int i = 0; i < dr.rowCount(); i++) {
            if (dr.toString(i, "PRINT2") == dr.toString(i, "PRINT1")) {
                continue;
            }
            if (dr.toString(i, "PRINT2") == prn) {
                QMap<QString, QString> d;
                d["qty"] = float_str(dr.toDouble(i, "QTY"), 1);
                d["dish"] = dr.toString(i, "NAME");
                d["comments"] = dr.toString(i, "COMMENTS");
                dishes.append(d);
            }
        }
        if (dishes.count() > 0) {
            MTPrintKitchen *p = new MTPrintKitchen(data, dishes, true, nullptr);
            p->start();
        }
    }
    fDb.open();
    for (int i = 0; i < dr.rowCount(); i++) {
        if (dr.value(i, "REMIND").toInt() > 0) {
            v[":date_start"] = QDateTime::currentDateTime();
            v[":staff_id"] = dh.value("STAFF_ID");
            v[":table_id"] = dh.value("TABLE_ID");
            v[":dish_id"] = dr.value(i, "DISH_ID");
            v[":qty"] = dr.value(i, "QTY");
            fDb.insert("o_dishes_reminder", v);
        }
    }
    v[":order_id"] = order;
    v[":state_id"] = DISH_STATE_NORMAL;
    fDb.select("update o_dishes set printed_qty=qty where order_id=:order_id and state_id=:state_id", v, dr);
    QJsonObject obj;
    obj["reply"] = "ok";
    return jsonreply(obj);
}

QByteArray MJsonHandler::handleReceipt(const QJsonObject &o)
{
    QString order = o["order"].toString();
    if (order.isEmpty()) {
        return jsonError(tr("Order cannot be empty"));
    }
    QPrinterInfo pi;
#ifndef QT_DEBUG
    if (!pi.availablePrinterNames().contains(o["printer"].toString())) {
        return jsonError(tr("Incorrect printer name") + "\r\n" + o["printer"].toString());
    }
#endif

    QMap<QString, QVariant> v;
    DatabaseResult dr;
    v[":id"] = order;
    fDb.select("select print_qty from o_order where id=:id", v, dr, false);
    if (dr.rowCount() == 0) {
        return jsonError(tr("Incorrect order number"));
    }
    if (dr.value("PRINT_QTY").toInt() > 0) {
        /* Check for staff privileges */
        int userId = fSessions[o["session"].toString()];
        if (userId == 0) {
            userId = o["staff"].toString().toInt();
        }
        /* Check for time , after 30 minutes deny any print */
        DatabaseResult dct;
        v[":f_order"] = order;
        MTFileLog::createLog(__LOG_RECEIPT, order + ", mark 2");
        fDb.select("select f_date, f_time from o_log where f_order=:f_order and f_action='Print prepayment check'", v, dct, false);
        if (dct.rowCount() > 0 && userId != 1) {
            if (QDate::fromString(dct.toString(0, "F_DATE"), "yyyy-MM-dd") != QDate::currentDate()) {
                return jsonError(tr("Cannot print receipt, becouse date is not equal current date"));
            }
            qDebug() << QTime::fromString(dct.toString(0, "F_TIME"), "HH:mm:ss").secsTo(QTime::currentTime());
            if (QTime::fromString(dct.toString(0, "F_TIME"), "HH:mm:ss").secsTo(QTime::currentTime()) > 60 * 30)  {
                return jsonError(tr("Cannot print receipt, becouse time up"));
            }
        }
        v[":id"] = userId;
        DatabaseResult dp;
        fDb.select("select group_id from employes where id=:id", v, dp, false);
        if (dp.rowCount() == 0) {
            return jsonError(tr("Cannot print multiple receipt\r for this order by you"));
        }
        if (dp.value("GROUP_ID").toInt() != 2 && dp.value("GROUP_ID").toInt() != 1) {
            return jsonError(tr("Cannot print multiple receipt\r for this order by you"));
        }
        //v[":fid"] = order;
        //db.select("delete from o_tax where fid=:fid", v, dp, false);
    }
    v[":id"] = order;
    fDb.select("select t.name as tname, h.name as hname, e.fname || ' ' || e.lname as ename, "
              "o.date_cash, o.amount, o.amount_inc, o.amount_dec, o.amount_inc_value, o.amount_dec_value "
              "from o_order o "
              "left join h_table t on t.id=o.table_id "
              "left join employes e on e.id=o.staff_id "
              "left join h_hall h on h.id=t.hall_id "
              "where o.id=:id ", v, dr, false);
    if (dr.rowCount() == 0) {
        return jsonError(tr("Unable to select order with id: ") + order);
    }
    double total;
    QMap<QString, QString> data;
    data["table"] = dr.value("TNAME").toString();
    data["hall"] = dr.value("HNAME").toString();
    data["staff"] = dr.value("ENAME").toString();
    data["date"] = dr.value("DATE_CASH").toDate().toString(def_date_format);
    data["printer"] = o["printer"].toString();
    data["order"] = order;
    total = dr.value("AMOUNT").toDouble();
    data["amount"] = float_str(total, 2);
    data["amount_inc"] = float_str(dr.value("AMOUNT_INC").toDouble(), 2);
    data["amount_dec"] = float_str(dr.value("AMOUNT_DEC").toDouble(), 2);
    data["amount_inc_value"] = float_str(dr.value("AMOUNT_INC_VALUE").toDouble() * 100, 2);
    data["amount_dec_value"] = float_str(dr.value("AMOUNT_DEC_VALUE").toDouble() * 100, 2);

    v[":f_order"] = order;
    fDb.select("select f_code, f_amount from o_gift_card where f_order=:f_order", v, dr, false);
    if (dr.rowCount() > 0) {
        double back  = 0;
        QString code = dr.value("F_CODE").toString();
        MSqlDatabase dbb("10.1.0.2", "maindb", "SYSDBA", "masterkey");
        if (!dbb.isOpen()) {
            return jsonError("Cannot connect to main server to complete gift order!");
        }
        DatabaseResult drg;
        v[":f_order"] = order;
        dbb.select("delete from o_gift_card where f_order=:f_order", v, drg);
        v[":f_code"] = code;
        dbb.select("select sum(f_amount) as f_amount, from o_gift_card where f_code=:f_code", v, drg);
        if (drg.rowCount() == 0) {
            return jsonError(tr("Invalid gift card"));
        }
        double balance = drg.toDouble(0, "f_amount");
        data["gift_before"] = float_str(balance, 2);
        if (total <= balance) {
            back = balance - total;
            v[":f_amount"] = total * -1;
            data["gift_used"] = float_str(total, 2);
        } else {
            back = 0;
            v[":f_amount"] = (total - balance) * -1;
            data["gift_used"] = float_str(total - balance, 2);
        }
        data["gift_after"] = float_str(back, 2);
        v[":f_order"] = order;
        fDb.select("update o_gift_card set f_amount=:f_amount where f_order=:f_order", v, dr, false);

        v[":f_code"] = code;
        v[":f_order"] = order;
        v[":f_amount"] = total * -1;
        dbb.select("insert into o_gift_card (f_code, f_order, f_amount) values (:f_code, :f_order, :f_amount)", v, drg);
        dbb.close();
    }

    QList<QMap<QString, QString> > dishes;
    v[":order_id"] = order;
    v[":state_id"] = DISH_STATE_NORMAL;
    fDb.select("select d.name, od.qty, od.price, od.qty*od.price as total, "
              "od.qty-od.printed_qty as printed,mt.adgcode, d.id as dishid, "
              "od.payment_mod as pm "
              "from o_dishes od "
              "left join me_dishes d on d.id=od.dish_id "
              "left join me_types mt on mt.id=d.type_id "
              "where od.order_id=:order_id and od.state_id=:state_id ",
              v, dr, false);
    for (int i = 0; i < dr.rowCount(); i++) {
        if (dr.value(i, "PRINTED").toDouble() > 0.1) {
            return jsonError(tr("Incomplete order"));
        }
        QMap<QString, QString> dish;
        dish["dish"] = dr.value(i, "NAME").toString();
        dish["qty"] = float_str(dr.value(i, "QTY").toDouble(), 1);
        dish["price"] = float_str(dr.value(i, "PRICE").toDouble(), 1);
        dish["total"] = float_str(dr.value(i, "TOTAL").toDouble(), 1);
        dish["adgcode"] = dr.value(i, "ADGCODE").toString();
        dish["id"] = dr.value(i, "DISHID").toString();
        dish["pm"] = dr.value(i, "PM").toString();
        dishes.append(dish);
    }

    bool nh = false;
    v[":id"] = order;
    DatabaseResult doh;
    fDb.select("select state_id from o_order where id=:id", v, doh, false);
    if (doh.rowCount() == 0) {
        return jsonError("Error. no order id");
    }

    double card = o["card"].toString().toDouble();
    double cash = o["cash"].toString().toDouble();
    nh = doh.value("STATE_ID").toInt() == 1;

    QJsonObject obj;
    DatabaseResult dpay;
    QString sn, firm, address, fiscal, hvhh, rseq, devnum, dept, time;
    v[":fid"] = order;
    fDb.select("select * from o_tax where fid=:fid", v, dpay, false);
    if (dpay.rowCount() == 0) {
        nh = true;
    } else {
        if (card != dpay.value("FCARD").toDouble()) {
            obj["warm"] = tr("Tax back number") + "\r\n" + dpay.value("FNUMBER").toString();
            v[":fid"] = order;
            DatabaseResult dtemp;
            fDb.select("delete from o_tax where fid=:fid", v, dtemp, false);
            nh = true;
        } else {
            PrintTaxN::parseResponse(dpay.value("JSON").toString(), firm, hvhh, fiscal, rseq, sn, address, devnum, time);
            MTFileLog::createLog(__LOG_RECEIPT, order + ", mark 7-3");
            data["sn"] = sn;
            data["firm"] = firm;
            data["address"] = address;
            data["fiscal"] = fiscal;
            data["taxnumber"] = QString("%1").arg(rseq.toInt(), 8, 10, QChar('0'));
            data["hvhh"] = hvhh;
            data["gh"] = devnum;
            data["taxtime"] = time;
            data["dept"] = __cnfmaindb.fTaxDepartment;
            nh = false;
        }
    }

    if (nh) {
        double serv = data["amount_inc_value"].toDouble();
        double disc = data["amount_dec_value"].toDouble();
        //int mode = o["mode"].toString().toInt();
        PrintTaxN pt(__cnfapp.taxIP(), __cnfapp.taxPort(), __cnfapp.taxPassword(), "true", this);
        for (int i = 0; i < dr.rowCount(); i++) {
            double price = dishes.at(i)["price"].toDouble();
            if (price < 1) {
                continue;
            }
            double tempDisc = 0.00;
            if (dishes.at(i)["pm"].toInt() == PaymentServiceDiscount) {
                tempDisc = disc;
            }
            pt.addGoods(__cnfmaindb.fTaxDepartment, dishes.at(i)["adgcode"], dishes.at(i)["id"], dishes.at(i)["dish"], price, dishes.at(i)["qty"].toDouble(), tempDisc);
        }
        if (data["amount_inc"].toDouble() > 0.001) {
            pt.addGoods(__cnfmaindb.fTaxDepartment, dishes.at(0)["adgcode"], "1", tr("Service"), data["amount_inc"].toDouble(), 1.0, disc);
        }
        QString jsonIn, jsonOut, err;
        int result = 0;
        result = pt.makeJsonAndPrint(card, 0, jsonIn, jsonOut, err);
        v[":forder"] = order;
        v[":fdate"] = QDateTime::currentDateTime();
        v[":fin"] = jsonIn;
        v[":fout"] = jsonOut;
        v[":ferr"] = err;
        v[":fresult"] = result;
        if (!fDb.insert("o_tax_log", v)) {
            v[":forder"] = order;
            v[":fdate"] = QDateTime::currentDateTime();
            v[":fin"] = fDb.lastError();
            v[":fout"] = fDb.lastError();
            v[":ferr"] = err;
            v[":fresult"] = -999;
            fDb.insert("o_tax_log", v);
        }
        if (result != pt_err_ok) {
           return jsonError(tr("Tax print error") + "\r\n" + err + jsonOut);
        } else {
            MTFileLog::createLog(__LOG_RECEIPT, order + ", mark 8-4");
            PrintTaxN::parseResponse(jsonOut, firm, hvhh, fiscal, rseq, sn, address, devnum, time);
            MTFileLog::createLog(__LOG_RECEIPT, order + ", mark 8-5");

            v[":fid"] = order;
            v[":ffiscal"] = fiscal;
            v[":fnumber"] = rseq;
            v[":fhvhh"] = hvhh;
            v[":fcash"] = cash;
            v[":fcard"] = card;
            v[":json"] = jsonOut;
            if (!fDb.insert("o_tax", v)) {
                return jsonError(fDb.lastError());
            }

            data["sn"] = sn;
            data["firm"] = firm;
            data["address"] = address;
            data["fiscal"] = fiscal;
            data["taxnumber"] = QString("%1").arg(rseq.toInt(), 8, 10, QChar('0'));
            data["hvhh"] = hvhh;
            data["gh"] = devnum;
            data["taxtime"] = time;
            data["dept"] = __cnfmaindb.fTaxDepartment;

            if (address.length() == 0) {
                return jsonError(tr("Firm address is empty"));
            }
        }
    }

    v[":id"] = order;
    fDb.select("update o_order set print_qty=abs(print_qty)+1 where id=:id", v, dr, false);

    MTPrintKitchen *m = new MTPrintKitchen(data, dishes, false, nullptr);
    m->start();
    obj["reply"] = "ok";
    return jsonreply(obj);
}

QByteArray MJsonHandler::handleDishInfo(const QJsonObject &o)
{
    int dish = o["dish"].toString().toInt();
    return QString("%1").arg(dish).toUtf8();
}

QByteArray MJsonHandler::handleSettings(const QJsonObject &o)
{
    Q_UNUSED(o);
    DatabaseResult dr;
    QMap<QString, QVariant> v;
    fDb.select("select id, name, price_inc, menu_id from sys_mobile_settings", v, dr);
    QJsonArray ja;
    for (int i = 0; i < dr.rowCount(); i++) {
        QJsonObject obj;
        obj["id"] = dr.value(i, "ID").toString();
        obj["name"] = dr.value(i, "NAME").toString();
        obj["priceinc"] = dr.value(i, "PRICE_INC").toString();
        obj["menuid"] = dr.value(i, "MENU_ID").toString();
        ja.append(obj);
    }
    QJsonObject jReply;
    jReply["reply"] = "ok";
    jReply["list"] = ja;
    return jsonreply(jReply);
}

QByteArray MJsonHandler::handleListReceipt(const QJsonObject &o)
{
    Q_UNUSED(o);
    DatabaseResult dr;
    QMap<QString, QVariant> v;
    fDb.select("select show_name, printer_name from sys_mobile_receipt_printers order by show_name", v, dr);
    QJsonArray ja;
    for (int i = 0; i < dr.rowCount(); i++) {
        QJsonObject obj;
        obj["showname"] = dr.value(i, "SHOW_NAME").toString();
        obj["printername"] = dr.value(i, "PRINTER_NAME").toString();
        ja.append(obj);
    }
    QJsonObject jReply;
    jReply["reply"] = "ok";
    jReply["type"] = "receiptprinters";
    jReply["list"] = ja;
    return jsonreply(jReply);
}

QByteArray MJsonHandler::handleCloseOrder(const QJsonObject &o)
{
    DatabaseResult dr;
    QMap<QString, QVariant> v;
    QString order = o["order"].toString();
    if (order.isEmpty()) {
        return jsonError(tr("Order is empty"));
    }
    v[":order_id"] = order;
    v[":state_id"] = DISH_STATE_NORMAL;
    fDb.select("select sum(qty)-sum(printed_qty) as printed from o_dishes where order_id=:order_id and state_id=:state_id", v, dr);
    if (dr.value("PRINTED").toDouble() > 0.1) {
        return jsonError(tr("Order is incomplete"));
    }
    v[":id"] = order;
    fDb.select("select staff_id, state_id, print_qty from o_order where id=:id ", v, dr);
    if (dr.rowCount() == 0) {
        return jsonError(tr("Incorrect order number"));
    }
    if (dr.value("STATE_ID").toInt() != ORDER_STATE_OPEN) {
        return jsonError(tr("Order is not opened"));
    }
    if (dr.value("PRINT_QTY") < 1) {
        return jsonError(tr("Receipt wasnt printed"));
    }
    int userId = fSessions[o["session"].toString()];
    if (userId == 0) {
        if (o.contains("staff")) {
            if (dr.value("STAFF_ID").toInt() != o["staff"].toString().toInt()) {
                return jsonError(tr("You are not owner of this order"));
            }
        } else {
            return jsonError(tr("You are not owner of this order"));
        }
    } else {
        if (userId != dr.value("STAFF_ID").toInt()) {
            return jsonError(tr("You are not owner of this order"));
        }
    }
    int payment = o["mode"].toString().toInt();
    v[":id"] = order;
    v[":state_id"] = ORDER_STATE_CLOSED;
    v[":payment"] = payment;
    v[":taxprint"] = -1;
    fDb.select("update o_order set state_id=:state_id, payment=:payment, taxprint=:taxprint where id=:id", v, dr);
    v[":order_id"] = order;
    fDb.select("update h_table set order_id='', lock_host=null where order_id=:order_id", v, dr);
    QJsonObject jReply;
    jReply["reply"] = "ok";
    jReply["orderclosed"] = "ok";

    v[":f_order"] = order;
    fDb.select("select f_code from o_gift_card where f_order=:f_order", v, dr);
    if (dr.rowCount() > 0) {
        MSqlDatabase dbb("10.1.0.2", "maindb", "SYSDBA", "masterkey");
        if (!dbb.isOpen()) {
            return jsonError("Cannot connect to main server to complete gift order!");
        }
        v[":f_code"] = dr.toString(0, "F_CODE");
        dbb.select("select sum(f_amount) from o_gift_card where f_code=:f_code", v, dr);
        if (dr.rowCount() == 0) {
            return jsonError(tr("Cannot process gift card, incorrect code"));
        }
    }

    return jsonreply(jReply);
}

QByteArray MJsonHandler::handleDishComment(const QJsonObject &o)
{
    int recid = o["dish"].toString().toInt();
    if (recid == 0) {
        return jsonError(tr("Incorrect dish record id"));
    }
    DatabaseResult dr;
    QMap<QString, QVariant> v;
    QJsonObject jReply;
    if (o.contains("set")) {
        v[":comments"] = o["set"].toString();
        fDb.update("o_dishes", v, recid);
    } else {
        v[":id"] = recid;
        fDb.select("select comments, printed_qty as qty from o_dishes where id=:id", v, dr);
        if (dr.rowCount() == 0) {
            return jsonError(tr("Incorrect dish record id"));
        }
        jReply["comment"] = dr.value("COMMENTS").toString();
        if (dr.value("QTY").toDouble() > 0.1) {
            jReply["readonly"] = "true";
        }
    }
    jReply["reply"] = "ok";
    return jsonreply(jReply);
}

QByteArray MJsonHandler::handleCommentsList(const QJsonObject &o)
{
    Q_UNUSED(o)
    DatabaseResult dr;
    QMap<QString, QVariant> v;
    QJsonObject jReply;
    QJsonArray ja;
    fDb.select("select name from me_dishes_comment order by queue", v, dr);
    for (int i = 0; i < dr.rowCount(); i++) {
        QJsonObject obj;
        obj["name"] = dr.value(i, "NAME").toString();
        ja.append(obj);
    }
    jReply["reply"] = "ok";
    jReply["commentslist"] = ja;
    return jsonreply(jReply);
}

QByteArray MJsonHandler::handleReadyDishes(const QJsonObject &o)
{
    Q_UNUSED(o)
    DatabaseResult dr;
    QMap<QString, QVariant> v;
    QJsonObject jReply;
    QJsonArray ja;
    fDb.select("select dr.id, t.name as tablename, e.fname || ' ' || e.lname as staff, m.name, dr.qty "
              "from o_dishes_reminder dr, h_table t, employes e, me_dishes m "
              "where dr.table_id=t.id and dr.staff_id=e.id and dr.dish_id=m.id "
              "and dr.date_ready is not null and dr.date_done is null "
              "order by dr.date_ready", v, dr);
    for (int i = 0; i < dr.rowCount(); i++) {
        QJsonObject obj;
        obj["id"] = dr.value(i, "ID").toString();
        obj["dish"] = dr.value(i, "NAME").toString();
        obj["table"] = dr.value(i, "TABLENAME").toString();
        obj["staff"] = dr.value(i, "STAFF").toString();
        obj["qty"] = float_str(dr.value(i, "QTY").toDouble(),1);
        ja.append(obj);
    }
    jReply["list"] = ja;
    jReply["reply"] = "ok";
    return jsonreply(jReply);
}

QByteArray MJsonHandler::handleReadyDishDone(const QJsonObject &o)
{
    if (!o.contains("id")) {
        return jsonError("Invalid dish record id");
    }
    QMap<QString, QVariant> v;
    QJsonObject jReply;
    v[":date_done"] = QDateTime::currentDateTime();
    fDb.update("o_dishes_reminder", v, o["id"].toString().toInt());
    jReply["reply"] = "ok";
    jReply["readydishdone"] = "";
    jReply["id"] = o["id"].toString();
    return jsonreply(jReply);
}

QByteArray MJsonHandler::handleMyMoney(const QJsonObject &o)
{
    QMap<QString, QVariant> v;
    QJsonObject jReply;
    v[":date_cash"] =  "'" + QDate::currentDate().toString("dd.MM.yyyy") + "'";
    v[":state_id"] = ORDER_STATE_CLOSED;
    v[":staff_id"] = fSessions[o["session"].toString()];
    DatabaseResult dr;
    fDb.select("select count(id) as QNT, coalesce(sum(amount), 0) as AMOUNT from o_order where date_cash=:date_cash and state_id=:state_id and staff_id=:staff_id", v, dr);
    if (dr.rowCount()) {
        jReply["reply"] = "ok";
        jReply["qty"] = dr.value("QNT").toString();
        jReply["mymoney"] = dr.value("AMOUNT").toString();
        jReply["mymoney"] = dr.value("QNT").toString() + " / " + dr.value("AMOUNT").toString();
    }
    return jsonreply(jReply);
}

QByteArray MJsonHandler::handleGetReminders(const QJsonObject &o)
{
    QMap<QString, QVariant> v;
    DatabaseResult dr;
    fDb.select("select r.record_id,"
               "lpad(extract(hour from cast(r.date_register as time)),2,'0') || ':' || lpad(extract(minute from cast(r.date_register as time)),2,'0') as reg_time, "
               "t.name as table_name, "
               "e.lname || ' ' || e.fname as staff_name, d.name as dish_name, r.qty, "
               "od.comments "
               "from o_dishes_reminder r "
               "inner join h_table t on t.id=r.table_id "
               "inner join employes e on e.id=r.staff_id "
               "inner join me_dishes d on d.id=r.dish_id "
               "inner join o_dishes od on od.id=r.record_id "
               "where r.state_id=0 "
               "order by r.id ", v, dr);
    QJsonArray ja;
    for (int i = 0; i < dr.rowCount(); i++) {
        QJsonObject jo;
        jo["rec"] = dr.toString(i, "RECORD_ID");
        jo["time"] = dr.toString(i, "REG_TIME");
        jo["table"] = dr.toString(i, "TABLE_NAME");
        jo["staff"] = dr.toString(i, "STAFF_NAME");
        jo["qty"] = dr.toString(i, "QTY");
        jo["state"] = 0;
        jo["dish"] = dr.toString(i, "DISH_NAME");
        jo["comment"] = dr.toString(i, "COMMENTS");
        ja.append(jo);
    }
    QJsonObject jreply;
    jreply["reply"] = "ok";
    jreply["dishes"] = ja;
    return jsonreply(jreply);
}

QByteArray MJsonHandler::handleReminder(const QJsonObject &o)
{
    QMap<QString, QVariant> v;
    v[":record_id"] = o["rec"].toString();
    v[":state_id"] = o["state"].toInt();
    DatabaseResult dr;
    switch (o["state"].toInt()) {
    case 0:
        break;
    case 1:
        fDb.select("update o_dishes_reminder set state_id=:state_id where record_id=:record_id", v, dr);
        break;
    case 2:
        v[":date_start"] = QDateTime::fromString(o["started"].toString(), "dd.MM.yyyy HH:mm:ss");
        fDb.select("update o_dishes_reminder set date_start=:date_start, state_id=:state_id where record_id=:record_id", v, dr);
        break;
    case 3:
        v[":date_ready"] = QDateTime::fromString(o["ready"].toString(), "dd.MM.yyyy HH:mm:ss");
        fDb.select("update o_dishes_reminder set date_ready=:date_ready, state_id=:state_id where record_id=:record_id", v, dr);
        break;
    }
    QJsonObject jo;
    jo["reply"] = "ok";
    jo["rec"] = o["rec"];
    jo["state"] = o["state"];
    return jsonreply(jo);
}

QString MJsonHandler::updateOrderAmount(const QString &id)
{
    QMap<QString, QVariant> v;
    v[":id"] = id;
    v[":state_id"] = DISH_STATE_NORMAL;
    DatabaseResult dr;
    fDb.select("select sum(qty*price) as total, sum(qty*price*price_inc) as totalinc, "
              "sum(((qty*price) + (qty*price*price_inc)) * price_dec) as totaldec "
              "from o_dishes where order_id=:id and state_id=:state_id", v, dr, false);
    double total = 0;
    if (dr.rowCount() > 0) {
        total = dr.value("TOTAL").toDouble() + dr.value("TOTALINC").toDouble() - dr.value("TOTALDEC").toDouble();
        v[":amount"] = total;
        v[":amount_inc"] = dr.value("TOTALINC");
        v[":amount_dec"] = dr.value("TOTALDEC");
        fDb.update("o_order", v, id);
    }
    return float_str(total, 0);
}

int MJsonHandler::checkPassword(const QString &pwd)
{
    QString pwdCrypt = QString(QCryptographicHash::hash(pwd.toLatin1(), QCryptographicHash::Md5).toHex());
    QMap<QString, QVariant> v;
    v[":password2"] = pwdCrypt;
    QString sql = "select id, group_id, fname || ' ' || lname as fullname, group_id "
            "from employes where password2=:password2 and state_id=1";
    DatabaseResult dr;
    fDb.select(sql, v, dr);
    if (dr.rowCount() == 0) {
        return 0;
    }

    return dr.value("ID").toInt();
}

QString MJsonHandler::sessionNew()
{
    return QUuid::createUuid().toString();
}

QByteArray MJsonHandler::jsonError(const QString &err)
{
    QJsonObject o;
    o["reply"] = err;
    return jsonreply(o);
}
