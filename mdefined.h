#ifndef MDEFINED_H
#define MDEFINED_H

#include <QDebug>
#include <QDate>
#include <QDateTime>
#include <QJsonDocument>
#include <QSettings>
#include <QString>

#define jsonreply(x) QJsonDocument(x).toJson()
#define def_date_format "dd.MM.yyyy"
#define def_date_time_format "dd.MM.yyyy HH:mm:ss"
#define float_str(value, f) QString::number(value, 'f', f).remove(QRegExp("\\.0+$")).remove(QRegExp("\\.$"))
#define __set_val(x) QSettings("Jazzve", "Jazzve").value(x)

#define ORDER_STATE_OPEN 1
#define ORDER_STATE_CLOSED 2
#define ORDER_STATE_EMTPY1 3 //Full empty
#define ORDER_STATE_EMPTY2 4 //Contain printed and then removed dishes
#define ORDER_STATE_REMOVED 5
#define ORDER_STATE_MERGE 6

#define DISH_STATE_NORMAL 1
#define DISH_STATE_REMOVED_NORMAL 2
#define DISH_STATE_REMOVED_PRINTED 3
#define DISH_STATE_MOVED 4
#define DISH_STATE_MOVE_PARTIALY 5
#define DISH_STATE_REMOVE_AFTER_CHECKOUT 6


#endif // MDEFINED_H
