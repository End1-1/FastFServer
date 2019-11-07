#-------------------------------------------------
#
# Project created by QtCreator 2019-05-22T17:02:04
#
#-------------------------------------------------

QT       += core gui network sql xml printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FastFServer
TEMPLATE = app

RC_FILE = res.rc

ICON = app.ico

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        ../LibQREncode/bitstream.c \
        ../LibQREncode/mask.c \
        ../LibQREncode/mmask.c \
        ../LibQREncode/mqrspec.c \
        ../LibQREncode/qrenc.c \
        ../LibQREncode/qrencode.c \
        ../LibQREncode/qrinput.c \
        ../LibQREncode/qrspec.c \
        ../LibQREncode/rscode.c \
        ../LibQREncode/split.c \
        ../NewTax/Src/printtaxn.cpp \
        QRCodeGenerator.cpp \
        c5printing.cpp \
        cnfapp.cpp \
        cnfmaindb.cpp \
        databaseresult.cpp \
        dbmutexlocker.cpp \
        dlgconnection.cpp \
        genqrcode.cpp \
        main.cpp \
        dialog.cpp \
        mdefined.cpp \
        mjsonhandler.cpp \
        msqldatabase.cpp \
        mtcpserver.cpp \
        mtfilelog.cpp \
        mtprintkitchen.cpp \
        phoneclientthread.cpp \
        phonejson.cpp \
        phonejson/phonejsonbase.cpp \
        phonejson/phonejsondishreminders.cpp \
        phonejson/phonejsonreminderstate.cpp \
        phonethread.cpp \
        pimage.cpp \
        pprintscene.cpp \
        ptextrect.cpp \
        qsystem.cpp \
        sqlthread.cpp

HEADERS += \
        ../LibQREncode/bitstream.h \
        ../LibQREncode/config.h \
        ../LibQREncode/getopt.h \
        ../LibQREncode/mask.h \
        ../LibQREncode/mmask.h \
        ../LibQREncode/mqrspec.h \
        ../LibQREncode/qrencode.h \
        ../LibQREncode/qrencode_inner.h \
        ../LibQREncode/qrinput.h \
        ../LibQREncode/qrspec.h \
        ../LibQREncode/rscode.h \
        ../LibQREncode/split.h \
        ../NewTax/Src/printtaxn.h \
        QRCodeGenerator.h \
        c5printing.h \
        cnfapp.h \
        cnfmaindb.h \
        databaseresult.h \
        dbmutexlocker.h \
        dialog.h \
        dlgconnection.h \
        genqrcode.h \
        mdefined.h \
        mjsonhandler.h \
        msqldatabase.h \
        mtcpserver.h \
        mtfilelog.h \
        mtprintkitchen.h \
        pdefaults.h \
        phoneclientthread.h \
        phonejson.h \
        phonejson/phonejsonbase.h \
        phonejson/phonejsondishreminders.h \
        phonejson/phonejsonreminderstate.h \
        phonethread.h \
        pimage.h \
        pprintscene.h \
        ptextrect.h \
        qsystem.h \
        sqlthread.h

FORMS += \
        dialog.ui \
        dlgconnection.ui

INCLUDEPATH += ./phonejson
INCLUDEPATH += C:/projects/NewTax/Src
INCLUDEPATH += C:/OpenSSL-Win32/include
INCLUDEPATH += C:/OpenSSL-Win32/include/openssl

LIBS += -lVersion
LIBS += -lwsock32
LIBS += -LC:/OpenSSL-Win32/lib
LIBS += -lopenssl
LIBS += -llibcrypto

DEFINES += FASTF
DEFINES += _ORGANIZATION_=\\\"Jazzve\\\"
DEFINES += _APPLICATION_=\\\"FastFServer\\\"

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
