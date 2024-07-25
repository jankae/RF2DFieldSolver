QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    CustomWidgets/informationbox.cpp \
    CustomWidgets/pcbview.cpp \
    CustomWidgets/siunitedit.cpp \
    Scenarios/coplanardifferentialmicrostrip.cpp \
    Scenarios/coplanardifferentialstripline.cpp \
    Scenarios/coplanarmicrostrip.cpp \
    Scenarios/coplanarstripline.cpp \
    Scenarios/differentialmicrostrip.cpp \
    Scenarios/differentialstripline.cpp \
    Scenarios/microstrip.cpp \
    Scenarios/scenario.cpp \
    Scenarios/stripline.cpp \
    element.cpp \
    elementlist.cpp \
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
    CustomWidgets/informationbox.h \
    CustomWidgets/pcbview.h \
    CustomWidgets/siunitedit.h \
    Scenarios/coplanardifferentialmicrostrip.h \
    Scenarios/coplanardifferentialstripline.h \
    Scenarios/coplanarmicrostrip.h \
    Scenarios/coplanarstripline.h \
    Scenarios/differentialmicrostrip.h \
    Scenarios/differentialstripline.h \
    Scenarios/microstrip.h \
    Scenarios/scenario.h \
    Scenarios/stripline.h \
    element.h \
    elementlist.h \
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
    CustomWidgets/vertexEditDialog.ui \
    Scenarios/scenario.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES +=
