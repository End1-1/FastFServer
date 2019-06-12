#include "dialog.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{

#ifndef QT_DEBUG
    QStringList libPath = QCoreApplication::libraryPaths();
    libPath << "C:/FastF/";
    libPath << "C:/FastF/platforms";
    libPath << "C:/FastF/sqldrivers";
    libPath << "C:/FastF/printsupport";
    QCoreApplication::setLibraryPaths(libPath);
#endif

    QApplication a(argc, argv);
    Dialog w;
    w.showFullScreen();

    QTranslator t;
    t.load(":/fastfserver.qm");
    a.installTranslator(&t);

    return a.exec();
}
