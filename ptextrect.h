#ifndef PTEXTRECT_H
#define PTEXTRECT_H

#include "pdefaults.h"
#include <QGraphicsItem>
#include <QTextOption>
#include <QBrush>
#include <QPen>
#include <QFont>

class PTextRect : public QGraphicsItem
{
public:
    PTextRect(QGraphicsItem *parent = 0);
    PTextRect(PTextRect &pr, const QString &text, QGraphicsItem *parent = 0);
    PTextRect(qreal x, qreal y, qreal w, qreal h, const QString &text, PTextRect *pr = 0, const QFont &font = QFont("Times", 10), QGraphicsItem *parent = 0);
    ~PTextRect();
    void setRect(qreal x, qreal y, qreal w, qreal h);
    void setWrapMode(QTextOption::WrapMode mode);
    void setBorders(bool l, bool t, bool r, bool b);
    void setFont(QFont font);
    void setFontSize(int size);
    void setFontBold(bool v);
    void setTextAlignment(Qt::Alignment align);
    void setRectPen(const QPen &pen);
    void setTextPen(const QPen &pen);
    void setBrush(const QBrush &brush);
    void adjustRect();
    QPointF pos() const;
    qreal height() const;
    qreal textHeight() const;
protected:
    QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
private:
    QBrush fBgBrush;
    QPen fRectPen;
    QPen fTextPen;
    QRectF fRect;
    QTextOption fTextOption;
    QString fText;
    QFont fFont;
    bool lnLeft;
    bool lnTop;
    bool lnRight;
    bool lnBottom;
};

#endif // PTEXTRECT_H
