QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    CustomWidgets/examplebrowserdialog.cpp \
    CustomWidgets/informationbox.cpp \
    CustomWidgets/labeleditdialog.cpp \
    CustomWidgets/pcbview.cpp \
    CustomWidgets/pointseditdialog.cpp \
    CustomWidgets/siunitedit.cpp \
    element.cpp \
    elementlist.cpp \
    expression.cpp \
    label.cpp \
    labellist.cpp \
    parameterlist.cpp \
    gauss/gauss.cpp \
    laplace/laplace.cpp \
    laplace/lattice.c \
    laplace/worker.c \
    main.cpp \
    mainwindow.cpp \
    polygon.cpp \
    savable.cpp \
    unit.cpp \
    util.cpp

HEADERS += \
    CustomWidgets/examplebrowserdialog.h \
    CustomWidgets/informationbox.h \
    CustomWidgets/labeleditdialog.h \
    CustomWidgets/pcbview.h \
    CustomWidgets/pointseditdialog.h \
    CustomWidgets/siunitedit.h \
    element.h \
    elementlist.h \
    expression.h \
    label.h \
    labellist.h \
    parameterlist.h \
    gauss/gauss.h \
    json.hpp \
    laplace/laplace.h \
    laplace/lattice.h \
    laplace/tuple.h \
    laplace/worker.h \
    mainwindow.h \
    polygon.h \
    qpointervariant.h \
    savable.h \
    unit.h \
    util.h

FORMS += \
    CustomWidgets/examplebrowserdialog.ui \
    CustomWidgets/labeleditdialog.ui \
    CustomWidgets/pointseditdialog.ui \
    CustomWidgets/vertexEditDialog.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

REVISION = $$system(git rev-parse HEAD)
DEFINES += GITHASH=\\"\"$$REVISION\\"\"
DEFINES += FW_MAJOR=1 FW_MINOR=0 FW_PATCH=0

RESOURCES += \
    resources.qrc

DISTFILES +=
