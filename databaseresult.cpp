#include "databaseresult.h"
#include <QSqlRecord>
#include <QSqlError>

DatabaseResult::DatabaseResult()
{

}

void DatabaseResult::fetchResult(QSqlQuery &q)
{
    fFieldsMap.clear();
    fData.clear();
    QSqlRecord r = q.record();
    int fieldsCount = r.count();
    for (int i = 0; i < fieldsCount; i++) {
        fFieldsMap[r.fieldName(i)] = i;
    }
    while (q.next()) {
        fData.append(QList<QVariant>());
        QList<QVariant> &data = fData[fData.count() - 1];
        for (int i = 0; i < fieldsCount; i++) {
            data << q.value(i);
        }
    }
}

bool DatabaseResult::makeIn(int column, QString &result)
{
    result.clear();
    for (int i = 0, count = fData.count(); i < count; i++) {
        result += fData.at(0).at(column).toString() + ",";
    }
    if (!result.isEmpty()) {
        result.remove(result.length() - 1, 1);
    }
    return !result.isEmpty();
}

bool DatabaseResult::makeIn(const QString &column, QString &result)
{
    int col = fFieldsMap[column];
    return makeIn(col, result);
}

void DatabaseResult::getBindValues(int row, QMap<QString, QVariant> &bind)
{
    const QList<QVariant> &r = fData.at(row);
    for (QMap<QString, int>::const_iterator it = fFieldsMap.begin(); it != fFieldsMap.end(); it++) {
        bind[":" + it.key()] = r.at(it.value());
    }
}

void DatabaseResult::removeRow(int row)
{
    fData.removeAt(row);
}
