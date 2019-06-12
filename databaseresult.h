#ifndef DATABASERESULT_H
#define DATABASERESULT_H

#include <QSqlQuery>
#include <QList>
#include <QVariant>
#include <QMap>

class DatabaseResult
{
public:
    DatabaseResult();
    void fetchResult(QSqlQuery &q);
    QVariant singleResult(const QString &sql, const QMap<QString, QVariant> &bind = QMap<QString, QVariant>());
    inline QVariant value(int row, int column) { return fData.at(row).at(column); }
    inline QVariant value(int row, const QString &column) { return fData.at(row).at(fFieldsMap[column]); }
    inline QVariant value(const QString &column) { return fData.at(0).at(fFieldsMap[column]); }
    inline QString toString(int row, const QString &column) {return value(row, column).toString();}
    inline double toDouble(int row, const QString &column) {return value(row, column).toDouble();}
    inline int toInt(int row, const QString &column) {return value(row, column).toInt();}
    inline void setValue(int row, const QString &column, const QVariant &value) {fData[row][fFieldsMap[column]] = value;}
    bool makeIn(int column, QString &result);
    bool makeIn(const QString &column, QString &result);
    inline int rowCount() { return fData.count(); }
    void getBindValues(int row, QMap<QString, QVariant> &bind);
    void removeRow(int row);
private:
    QMap<QString, int> fFieldsMap;
    QList<QList<QVariant> > fData;

};

#endif // DATABASERESULT_H
