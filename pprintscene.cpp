#include "pprintscene.h"
#include "ptextrect.h"
#include "pimage.h"
#include <QGraphicsTextItem>

PPrintScene::PPrintScene(PrintOrientation po, QObject *parent) :
    QGraphicsScene(parent)
{
    if (po == Portrait) {
        setSceneRect(0, 0, 2100, 2970);
    } else {
        setSceneRect(0, 0, 2970, 2100);
    }
    fPrintOrientation = po;
}

PPrintScene::PPrintScene(QObject *parent) :
    QGraphicsScene(parent)
{
    setSceneRect(0, 0, 800, 2970);
}

PTextRect *PPrintScene::addTextRect(PTextRect *t)
{
    addItem(t);
    return t;
}

PTextRect *PPrintScene::addTextRect(qreal x, qreal y, qreal w, qreal h, const QString &text, PTextRect *tmpl)
{
    return addTextRect(new PTextRect(x, y, w, h, text, tmpl));
}

int PPrintScene::addTableRow(int &top, int height, QList<int> &col, QStringList &values, PTextRect *tmpl)
{
    int lastTop = height;
    int start = col.at(0);
    for (int i = 0; i < values.count(); i++) {
        int currHeight = addTextRect(new PTextRect(start, top, col.at(i + 1), height, values.at(i), tmpl))->textHeight();
        start += col.at(i + 1);
        lastTop = lastTop > currHeight ? lastTop : currHeight;
    }
    top += lastTop;
    values.clear();
    return top;
}
