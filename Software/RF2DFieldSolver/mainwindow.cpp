#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->view->setCorners(QPointF(-20, 5), QPointF(20, -2));

    showMaximized();
    ui->splitter->setSizes({1000, 3000, 1000});
    ui->splitter_2->setSizes({5000, 1000});

    ui->resolution->setUnit("m");
    ui->resolution->setPrefixes("um ");
    ui->resolution->setPrecision(4);
    ui->resolution->setValue(10e-6);

    ui->tolerance->setUnit("V");
    ui->tolerance->setPrefixes("pnum ");
    ui->tolerance->setPrecision(4);
    ui->tolerance->setValue(1e-6);

    list = new ElementList();
    ui->table->setModel(list);
    ui->table->setItemDelegateForColumn((int) ElementList::Column::Type, new TypeDelegate());
    ui->view->setElementList(list);

    // connections for adding/removing elements
    auto addMenu = new QMenu();
    auto addRF = new QAction("Trace");
    connect(addRF, &QAction::triggered, [=](){
        auto e = new Element(Element::Type::Trace);
        list->addElement(e);
        ui->view->startAppending(e);
    });
    addMenu->addAction(addRF);
    auto addDielectric = new QAction("Dielectric");
    connect(addDielectric, &QAction::triggered, [=](){
        auto e = new Element(Element::Type::Dielectric);
        list->addElement(e);
        ui->view->startAppending(e);
    });
    addMenu->addAction(addDielectric);
    auto addGND = new QAction("GND");
    connect(addGND, &QAction::triggered, [=](){
        auto e = new Element(Element::Type::GND);
        list->addElement(e);
        ui->view->startAppending(e);
    });
    addMenu->addAction(addGND);
    ui->add->setMenu(addMenu);

//    auto trace = new Element(Element::Type::Trace);
//    trace->appendVertex(QPointF(-2, 0));
//    trace->appendVertex(QPointF(1, 0));
//    trace->appendVertex(QPointF(1, 0.2));
//    trace->appendVertex(QPointF(-1, 0.2));

//    list->addElement(trace);

//    auto gnd = new Element(Element::Type::GND);
//    gnd->appendVertex(QPointF(-5, 0));
//    gnd->appendVertex(QPointF(5, 0));
//    gnd->appendVertex(QPointF(5, -0.5));
//    gnd->appendVertex(QPointF(-5, -0.5));

//    list->addElement(gnd);
}

MainWindow::~MainWindow()
{
    delete ui;
}

