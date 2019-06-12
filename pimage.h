#ifndef PIMAGE_H
#define PIMAGE_H

#include "pdefaults.h"
#include <QGraphicsItem>

class PImage : public QGraphicsItem
{
public:
    PImage(const QString &fileName, QGraphicsItem *parent = 0);
    ~PImage();
    void setRect(const QRectF &rect);
protected:
    QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
private:
    QString fImageName;
    QRectF fRect;
};

#endif // PIMAGE_H
