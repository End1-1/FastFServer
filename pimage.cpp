#include "pimage.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <qDebug>

PImage::PImage(const QString &fileName, QGraphicsItem *parent) :
    QGraphicsItem(parent)
{
    qDebug() << fileName;
    fImageName = fileName;
}

PImage::~PImage()
{
}

void PImage::setRect(const QRectF &rect)
{
    fRect = rect;
    update();
}

QRectF PImage::boundingRect() const
{
    return fRect;
}

void PImage::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    QRect r = option->rect;
    QImage image(fImageName);
    painter->drawImage(r, image);
}
