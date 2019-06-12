#ifndef PPRINTSCENE_H
#define PPRINTSCENE_H

#include "pdefaults.h"
#include <QGraphicsScene>

namespace PPrintOptions {
enum PrintOrientation {Portrait, Landscape};
}

using namespace PPrintOptions;

class PTextRect;
class PImage;

class PPrintScene : public QGraphicsScene
{
public:
    PPrintScene(PrintOrientation po, QObject *parent = nullptr);
    PPrintScene(QObject *parent = nullptr);
    PTextRect *addTextRect(PTextRect *t);
    PTextRect *addTextRect(qreal x, qreal y, qreal w, qreal h, const QString &text, PTextRect *tmpl);
    PrintOrientation fPrintOrientation;
    int addTableRow(int &top, int height, QList<int> &col, QStringList &values, PTextRect *tmpl);
private:

};

#endif // PPRINTSCENE_H
