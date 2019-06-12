#include "ptextrect.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>

PTextRect::PTextRect(QGraphicsItem *parent) :
    QGraphicsItem(parent)
{
    fFont = QFont("Times", 10);
    lnLeft = true;
    lnTop = true;
    lnRight = true;
    lnBottom = true;
    fTextOption.setAlignment(Qt::AlignVCenter);
    fTextOption.setWrapMode(QTextOption::NoWrap);
    fRectPen = QPen(Qt::black, Qt::DotLine);
    fRectPen.setWidthF(0.5);
}

PTextRect::PTextRect(PTextRect &pr, const QString &text, QGraphicsItem *parent) :
    QGraphicsItem(parent)
{
    fRect = pr.fRect;
    fText = text;
    fFont = pr.fFont;
    fBgBrush = pr.fBgBrush;
    fRectPen = pr.fRectPen;
    fTextPen = pr.fTextPen;
    lnLeft = pr.lnLeft;
    lnTop = pr.lnTop;
    lnRight = pr.lnRight;
    lnBottom = pr.lnBottom;
    fTextOption = pr.fTextOption;
}

PTextRect::PTextRect(qreal x, qreal y, qreal w, qreal h, const QString &text, PTextRect *pr,
                     const QFont &font, QGraphicsItem *parent) :
    QGraphicsItem(parent)
{
    fRect = QRectF(QPointF(x, y), QSizeF(w, h));
    fText = text;
    fFont = font;
    fTextOption.setAlignment(Qt::AlignVCenter);
    fTextOption.setWrapMode(QTextOption::NoWrap);
    if (pr) {
        fBgBrush = pr->fBgBrush;
        fRectPen = pr->fRectPen;
        fTextPen = pr->fTextPen;
        fTextOption = pr->fTextOption;
        fFont = pr->fFont;
        lnLeft = pr->lnLeft;
        lnTop = pr->lnTop;
        lnRight = pr->lnRight;
        lnBottom = pr->lnBottom;
        fTextOption = pr->fTextOption;
    } else {
        lnLeft = true;
        lnTop = true;
        lnRight = true;
        lnBottom = true;
    }
    adjustRect();
}

PTextRect::~PTextRect()
{

}

void PTextRect::setRect(qreal x, qreal y, qreal w, qreal h)
{
    fRect = QRectF(x, y, w, h);
}

void PTextRect::setWrapMode(QTextOption::WrapMode mode)
{
    fTextOption.setWrapMode(mode);
}

void PTextRect::setBorders(bool l, bool t, bool r, bool b)
{
    lnLeft = l;
    lnTop = t;
    lnRight = r;
    lnBottom = b;
}

void PTextRect::setFont(QFont font)
{
    fFont = font;
}

void PTextRect::setFontSize(int size)
{
    fFont.setPointSize(size);
}

void PTextRect::setFontBold(bool v)
{
    fFont.setBold(v);
}

void PTextRect::setTextAlignment(Qt::Alignment align)
{
    fTextOption.setAlignment(align);
}

void PTextRect::setRectPen(const QPen &pen)
{
    fRectPen = pen;
}

void PTextRect::setTextPen(const QPen &pen)
{
    fTextPen = pen;
}

void PTextRect::setBrush(const QBrush &brush)
{
    fBgBrush = brush;
}

void PTextRect::adjustRect()
{
    int h = fRect.height() + 20;
    QFontMetrics fm(fFont);
    QRect rect(fRect.left(), fRect.top(), fRect.width(), fRect.height());
    if (fTextOption.wrapMode() != QTextOption::NoWrap) {
        rect = fm.boundingRect(rect, Qt::TextWordWrap, fText);
        rect.setHeight(rect.height());
        if (rect.height() > h) {
            fRect = QRectF(QPointF(fRect.x(), fRect.y()), QSizeF(fRect.width(), rect.height() + fm.descent() + fm.ascent() + 2));
        } else if (rect.height() < h) {
            rect.setHeight(h);
        }
    } else {
        rect = fm.boundingRect(rect, Qt::TextSingleLine, fText);
        rect.setHeight(rect.height());
        if (rect.height() > h) {
            fRect = QRectF(QPointF(fRect.x(), fRect.y()), QSizeF(fRect.width(), rect.height() + fm.descent() + fm.ascent() + 2));
        } else if (rect.height() < h) {
            rect.setHeight(h);
        }
    }
}

QPointF PTextRect::pos() const
{
    return QPointF(fRect.left(), fRect.top());
}

qreal PTextRect::height() const
{
    return fRect.height();
}

qreal PTextRect::textHeight() const
{
    return fRect.height();
}

QRectF PTextRect::boundingRect() const
{
    return fRect;
}

void PTextRect::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    painter->fillRect(option->rect, fBgBrush);
    painter->setPen(fRectPen);
    if (lnLeft) {
        painter->drawLine(fRect.x(), fRect.y(), fRect.x(), fRect.bottom());
    }
    if (lnTop) {
        painter->drawLine(fRect.x(), fRect.y(), fRect.right(), fRect.y());
    }
    if (lnRight) {
        painter->drawLine(fRect.right(), fRect.y(), fRect.right(), fRect.bottom());
    }
    if (lnBottom) {
        painter->drawLine(fRect.x(), fRect.bottom(), fRect.right(), fRect.bottom());
    }
    QRect textRect = option->rect;
    textRect.adjust(8, 8, -8, -8);
    painter->setPen(fTextPen);
    painter->setFont(fFont);
    painter->drawText(textRect, fText, fTextOption);
}
