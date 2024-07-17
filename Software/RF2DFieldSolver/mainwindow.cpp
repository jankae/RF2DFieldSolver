#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QScrollBar>

#include <QDebug>
#include <QVector>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    auto updateViewArea = [=](){
        ui->view->setCorners(QPointF(ui->xleft->value(), ui->ytop->value()), QPointF(ui->xright->value(), ui->ybottom->value()));
        ui->view->update();
    };

    showMaximized();
    ui->splitter->setSizes({1000, 3000, 1000});
    ui->splitter_2->setSizes({5000, 1000});

    ui->resolution->setUnit("m");
    ui->resolution->setPrefixes("um ");
    ui->resolution->setPrecision(4);
    ui->resolution->setValue(50e-6);

    ui->tolerance->setUnit("V");
    ui->tolerance->setPrefixes("pnum ");
    ui->tolerance->setPrecision(4);
    ui->tolerance->setValue(2e-6);

    ui->threads->setValue(20);

    ui->borderIsGND->setChecked(true);

    ui->xleft->setUnit("m");
    ui->xleft->setPrefixes("um ");
    ui->xleft->setPrecision(4);
    connect(ui->xleft, &SIUnitEdit::valueChanged, this, updateViewArea);
    ui->xleft->setValue(-3e-3);

    ui->xright->setUnit("m");
    ui->xright->setPrefixes("um ");
    ui->xright->setPrecision(4);
    connect(ui->xright, &SIUnitEdit::valueChanged, this, updateViewArea);
    ui->xright->setValue(3e-3);

    ui->ytop->setUnit("m");
    ui->ytop->setPrefixes("um ");
    ui->ytop->setPrecision(4);
    connect(ui->ytop, &SIUnitEdit::valueChanged, this, updateViewArea);
    ui->ytop->setValue(3e-3);

    ui->ybottom->setUnit("m");
    ui->ybottom->setPrefixes("um ");
    ui->ybottom->setPrecision(4);
    connect(ui->ybottom, &SIUnitEdit::valueChanged, this, updateViewArea);
    ui->ybottom->setValue(-1e-3);

    ui->gridsize->setUnit("m");
    ui->gridsize->setPrefixes("um ");
    ui->gridsize->setPrecision(4);
    connect(ui->gridsize, &SIUnitEdit::valueChanged, this, [=](){
        ui->view->setGrid(ui->gridsize->value());
    });
    ui->gridsize->setValue(1e-4);

    connect(ui->showPotential, &QCheckBox::toggled, this, [=](bool enabled){
        ui->view->setShowPotential(enabled);
    });
    ui->showPotential->setChecked(true);

    connect(ui->showGrid, &QCheckBox::toggled, this, [=](bool enabled){
        ui->view->setShowGrid(enabled);
    });
    ui->showGrid->setChecked(true);

    connect(ui->snapGrid, &QCheckBox::toggled, this, [=](bool enabled){
        ui->view->setSnapToGrid(enabled);
    });
    ui->snapGrid->setChecked(true);

    ui->capacitance->setUnit("F");
    ui->capacitance->setPrefixes("fpnum ");
    ui->capacitance->setPrecision(4);

    ui->inductance->setUnit("H");
    ui->inductance->setPrefixes("fpnum ");
    ui->inductance->setPrecision(4);

    ui->impedance->setUnit("Ω");
    ui->impedance->setPrecision(4);

    list = new ElementList();
    ui->table->setModel(list);
    ui->table->setItemDelegateForColumn((int) ElementList::Column::Type, new TypeDelegate());
    ui->view->setElementList(list);
    ui->view->setLaplace(&laplace);

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

    connect(ui->remove, &QPushButton::clicked, this, [=](){
        auto row = ui->table->currentIndex().row();
        if(row >= 0 && row <= list->getElements().size()) {
            list->removeElement(row);
            ui->view->update();
        }
    });

    // connections for the calculations
    connect(&laplace, &Laplace::info, this, &MainWindow::info);
    connect(&laplace, &Laplace::warning, this, &MainWindow::warning);
    connect(&laplace, &Laplace::error, this, &MainWindow::error);
    connect(&laplace, &Laplace::calculationDone, this, [=](){
        // TODO
        ui->view->update();
        laplaceAir.setArea(ui->view->getTopLeft(), ui->view->getBottomRight());
        laplaceAir.setGrid(ui->resolution->value());
        laplaceAir.setThreads(ui->threads->value());
        laplaceAir.setThreshold(ui->tolerance->value());
        laplaceAir.setGroundedBorders(ui->borderIsGND->isChecked());
        laplaceAir.startCalculation(list);
        // start gauss calculation
        info("Starting gauss integration for charge with dielectric");
        double chargeSum = 0;
        for(auto e : list->getElements()) {
            if(e->getType() != Element::Type::Trace) {
                // ignore
                continue;
            }
            chargeSum += Gauss::getCharge(&laplace, list, e, ui->resolution->value());
        }
        info("Dielectric gauss calculation done");
        Cdielectric = chargeSum * e0;
        ui->capacitance->setValue(Cdielectric);
    });

    laplaceAir.setIgnoreDielectric(true);

    connect(&laplaceAir, &Laplace::info, this, &MainWindow::info);
    connect(&laplaceAir, &Laplace::warning, this, &MainWindow::warning);
    connect(&laplaceAir, &Laplace::error, this, &MainWindow::error);
    connect(&laplaceAir, &Laplace::calculationDone, this, [=](){
        // start gauss calculation
        info("Starting gauss integration for charge without dielectric");
        double chargeSum = 0;
        for(auto e : list->getElements()) {
            if(e->getType() != Element::Type::Trace) {
                // ignore
                continue;
            }
            chargeSum += Gauss::getCharge(&laplaceAir, nullptr, e, ui->resolution->value());
        }
        info("Air gauss calculation done");
        auto Cair = chargeSum * e0;
        auto L = 1.0 / (std::pow(2.998e8, 2.0) * Cair);
        ui->inductance->setValue(L);
        auto impedance = sqrt(L / Cdielectric);
        ui->impedance->setValue(impedance);
    });

    connect(ui->update, &QPushButton::clicked, this, [=](){
        // start the calculations
        ui->status->clear();
        ui->capacitance->setValue(0);
        ui->inductance->setValue(0);
        ui->impedance->setValue(0);
        // TODO sanity check elements
        laplace.setArea(ui->view->getTopLeft(), ui->view->getBottomRight());
        laplace.setGrid(ui->resolution->value());
        laplace.setThreads(ui->threads->value());
        laplace.setThreshold(ui->tolerance->value());
        laplace.setGroundedBorders(ui->borderIsGND->isChecked());
        laplace.startCalculation(list);
        ui->view->update();
    });

    // create standard elements
    auto trace = new Element(Element::Type::Trace);
    trace->appendVertex(QPointF(-0.15e-3, 0));
    trace->appendVertex(QPointF(0.15e-3, 0));
    trace->appendVertex(QPointF(0.15e-3, 35e-6));
    trace->appendVertex(QPointF(-0.15e-3, 35e-6));
    list->addElement(trace);

    auto substrate = new Element(Element::Type::Dielectric);
    substrate->appendVertex(QPointF(-3e-3, 0));
    substrate->appendVertex(QPointF(3e-3, 0));
    substrate->appendVertex(QPointF(3e-3, -0.2e-3));
    substrate->appendVertex(QPointF(-3e-3, -0.2e-3));
    list->addElement(substrate);

    auto gnd = new Element(Element::Type::GND);
    gnd->appendVertex(QPointF(-3e-3, -0.2e-3));
    gnd->appendVertex(QPointF(3e-3, -0.2e-3));
    gnd->appendVertex(QPointF(3e-3, -0.3e-3));
    gnd->appendVertex(QPointF(-3e-3, -0.3e-3));
    list->addElement(gnd);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::info(QString info)
{
    QTextCharFormat tf;
    tf = ui->status->currentCharFormat();
    tf.setForeground(QBrush(Qt::black));
    ui->status->setCurrentCharFormat(tf);
    ui->status->appendPlainText(info);
    QScrollBar *sb = ui->status->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void MainWindow::warning(QString warning)
{
    QTextCharFormat tf;
    tf = ui->status->currentCharFormat();
    tf.setForeground(QBrush(QColor(255, 174, 26)));
    ui->status->setCurrentCharFormat(tf);
    ui->status->appendPlainText(warning);
    QScrollBar *sb = ui->status->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void MainWindow::error(QString error)
{
    QTextCharFormat tf;
    tf = ui->status->currentCharFormat();
    tf.setForeground(QBrush(QColor(255, 94, 0)));
    ui->status->setCurrentCharFormat(tf);
    ui->status->appendPlainText(error);
    QScrollBar *sb = ui->status->verticalScrollBar();
    sb->setValue(sb->maximum());
}

