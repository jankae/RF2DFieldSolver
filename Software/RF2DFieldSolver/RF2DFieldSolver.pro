QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    CustomWidgets/pcbview.cpp \
    CustomWidgets/siunitedit.cpp \
    element.cpp \
    elementlist.cpp \
    gauss/gauss.cpp \
    laplace/laplace.cpp \
    laplace/lattice.c \
    laplace/worker.c \
    main.cpp \
    mainwindow.cpp \
    polygon.cpp \
    unit.cpp \
    util.cpp

HEADERS += \
    CustomWidgets/pcbview.h \
    CustomWidgets/siunitedit.h \
    element.h \
    elementlist.h \
    gauss/gauss.h \
    laplace/laplace.h \
    laplace/lattice.h \
    laplace/tuple.h \
    laplace/worker.h \
    mainwindow.h \
    polygon.h \
    unit.h \
    util.h

FORMS += \
    CustomWidgets/vertexEditDialog.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
