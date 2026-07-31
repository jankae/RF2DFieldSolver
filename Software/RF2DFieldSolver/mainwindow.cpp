#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QScrollBar>

#include <QDebug>
#include <QVector>
#include <QItemSelectionModel>

#include "polygon.h"

#include "CustomWidgets/pointseditdialog.h"

#include "Scenarios/scenario.h"

static const QString APP_VERSION = QString::number(FW_MAJOR) + "." +
                                   QString::number(FW_MINOR) + "." +
                                   QString::number(FW_PATCH);
static const QString APP_GIT_HASH = QString(GITHASH);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle(windowTitle() + " "+APP_VERSION+"-"+APP_GIT_HASH.left(9));

    auto updateViewArea = [=](){
        ui->view->setCorners(QPointF(ui->xleft->value(), ui->ytop->value()), QPointF(ui->xright->value(), ui->ybottom->value()));
        ui->view->update();
    };

    showMaximized();
    ui->splitter->setSizes({1000, 3000, 1000});
    ui->splitter_2->setSizes({5000, 1000});
    ui->leftSplitter->setSizes({1000, 2000});

    ui->resolution->setUnit("m");
    ui->resolution->setPrefixes("um ");
    ui->resolution->setPrecision(4);
    ui->resolution->setValue(10e-6);

    ui->gaussDistance->setUnit("m");
    ui->gaussDistance->setPrefixes("um ");
    ui->gaussDistance->setPrecision(4);
    ui->gaussDistance->setValue(20e-6);

    ui->tolerance->setUnit("V");
    ui->tolerance->setPrefixes("pnum ");
    ui->tolerance->setPrecision(4);
    ui->tolerance->setValue(100e-9);

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

    connect(ui->viewMode, &QComboBox::currentIndexChanged, this, [=](){
        ui->view->setKeepAspectRatio(ui->viewMode->currentIndex() == 0);
    });

    ui->capacitanceP->setUnit("F/m");
    ui->capacitanceP->setPrefixes("fpnum ");
    ui->capacitanceP->setPrecision(4);

    ui->inductanceP->setUnit("H/m");
    ui->inductanceP->setPrefixes("fpnum ");
    ui->inductanceP->setPrecision(4);

    ui->impedanceP->setUnit("Ω");
    ui->impedanceP->setPrecision(4);

    ui->capacitanceN->setUnit("F/m");
    ui->capacitanceN->setPrefixes("fpnum ");
    ui->capacitanceN->setPrecision(4);

    ui->inductanceN->setUnit("H/m");
    ui->inductanceN->setPrefixes("fpnum ");
    ui->inductanceN->setPrecision(4);

    ui->impedanceN->setUnit("Ω");
    ui->impedanceN->setPrecision(4);

    ui->impedanceDiff->setUnit("Ω");
    ui->impedanceDiff->setPrecision(4);

    // save/load
    connect(ui->actionOpen, &QAction::triggered, this, [=](){
        openFromFileDialog("Load project", "RF 2D field solver files (*.RF2Dproj)");
        ui->view->update();
    });
    connect(ui->actionSave, &QAction::triggered, this, [=](){
        saveToFileDialog("Load project", "RF 2D field solver files (*.RF2Dproj)", ".RF2Dproj");
    });

    list = new ElementList();
    ui->table->setModel(list);
    ui->table->setItemDelegateForColumn((int) ElementList::Column::Type, new TypeDelegate());
    ui->view->setElementList(list);
    ui->view->setLaplace(&laplace);

    // parameters
    params = new ParameterList();
    ui->paramTable->setModel(params);
    ui->view->setParameters(params);
    connect(ui->paramAdd, &QPushButton::clicked, this, [=](){
        params->addParameter();
    });
    connect(ui->paramRemove, &QPushButton::clicked, this, [=](){
        auto row = ui->paramTable->currentIndex().row();
        if(row >= 0) {
            params->removeParameter(row);
        }
    });
    // any parameter change re-evaluates the geometry and replots
    connect(params, &ParameterList::parametersChanged, this, [=](){
        refreshGeometry();
    });

    // edit the points of the selected element as expressions
    connect(ui->editPoints, &QPushButton::clicked, this, [=](){
        auto row = ui->table->currentIndex().row();
        if(row < 0 || row >= list->getElements().size()) {
            return;
        }
        // defining points manually ends any click-to-draw session in progress
        ui->view->stopAppending();
        auto e = list->elementAt(row);
        PointsEditDialog d(e, params->symbols(), this);
        if(d.exec() == QDialog::Accepted) {
            refreshGeometry();
        }
    });

    // connections for adding/removing elements
    auto addMenu = new QMenu();
    auto addRF = new QAction("Trace (+)");
    connect(addRF, &QAction::triggered, [=](){
        auto e = new Element(Element::Type::TracePos);
        list->addElement(e);
        ui->table->selectRow(list->getElements().size() - 1);
        ui->view->startAppending(e);
    });
    addMenu->addAction(addRF);
    auto addRFNeg = new QAction("Trace (-)");
    connect(addRFNeg, &QAction::triggered, [=](){
        auto e = new Element(Element::Type::TraceNeg);
        list->addElement(e);
        ui->table->selectRow(list->getElements().size() - 1);
        ui->view->startAppending(e);
    });
    addMenu->addAction(addRFNeg);
    auto addDielectric = new QAction("Dielectric");
    connect(addDielectric, &QAction::triggered, [=](){
        auto e = new Element(Element::Type::Dielectric);
        list->addElement(e);
        ui->table->selectRow(list->getElements().size() - 1);
        ui->view->startAppending(e);
    });
    addMenu->addAction(addDielectric);
    auto addGND = new QAction("GND");
    connect(addGND, &QAction::triggered, [=](){
        auto e = new Element(Element::Type::GND);
        list->addElement(e);
        ui->table->selectRow(list->getElements().size() - 1);
        ui->view->startAppending(e);
    });
    addMenu->addAction(addGND);
    ui->add->setMenu(addMenu);

    connect(ui->remove, &QPushButton::clicked, this, [=](){
        auto row = ui->table->currentIndex().row();
        if(row >= 0 && row <= list->getElements().size()) {
            list->removeElement(row);
            ui->view->setSelectedElement(nullptr);
            ui->view->update();
        }
    });

    // clicking an element in the view selects its row in the table
    connect(ui->view, &PCBView::elementSelected, this, [=](Element *e){
        if(e) {
            int row = list->getElements().indexOf(e);
            if(row >= 0) {
                ui->table->selectRow(row);
            }
        } else {
            ui->table->clearSelection();
            ui->view->setSelectedElement(nullptr);
        }
    });
    wireTableSelection();

    // connections for the calculations
    connect(ui->update, &QPushButton::clicked, this, &MainWindow::startCalculation);

    connect(&laplace, &Laplace::info, this, &MainWindow::info);
    connect(&laplace, &Laplace::warning, this, &MainWindow::warning);
    connect(&laplace, &Laplace::error, this, &MainWindow::error);
    connect(&laplace, &Laplace::calculationDone, this, [=](){
        // laplace is done
        disconnect(&laplace, &Laplace::percentage, this, nullptr);
        disconnect(ui->abort, nullptr, &laplace, nullptr);

        ui->view->update();
        // start gauss calculation
        info("Starting gauss integration for charge without dielectric");
        double chargeSumP = 0, chargeSumN = 0;
        for(auto e : list->getElements()) {
            switch(e->getType()) {
            case Element::Type::TracePos:
                chargeSumP += Gauss::getCharge(&laplace, nullptr, e, ui->resolution->value(), ui->gaussDistance->value());
                break;
            case Element::Type::TraceNeg:
                chargeSumN -= Gauss::getCharge(&laplace, nullptr, e, ui->resolution->value(), ui->gaussDistance->value());
                break;
            case Element::Type::GND:
            case Element::Type::Dielectric:
            case Element::Type::Last:
                break;
            }
        }
        info("Air gauss calculation done");
        auto CairP = chargeSumP * e0;
        auto LP = 1.0 / (std::pow(2.998e8, 2.0) * CairP);
        ui->inductanceP->setValue(LP);

        auto CairN = chargeSumN * e0;
        auto LN = 1.0 / (std::pow(2.998e8, 2.0) * CairN);
        ui->inductanceN->setValue(LN);

        // start gauss calculation
        info("Starting gauss integration for charge with dielectric");
        chargeSumP = 0, chargeSumN = 0;
        for(auto e : list->getElements()) {
            switch(e->getType()) {
            case Element::Type::TracePos:
                chargeSumP += Gauss::getCharge(&laplace, list, e, ui->resolution->value(), ui->gaussDistance->value());
                break;
            case Element::Type::TraceNeg:
                chargeSumN -= Gauss::getCharge(&laplace, list, e, ui->resolution->value(), ui->gaussDistance->value());
                break;
            case Element::Type::GND:
            case Element::Type::Dielectric:
            case Element::Type::Last:
                break;
            }
        }
        info("Dielectric gauss calculation done");
        auto CdielectricP = chargeSumP * e0;
        ui->capacitanceP->setValue(CdielectricP);

        auto CdielectricN = chargeSumN * e0;
        ui->capacitanceN->setValue(CdielectricN);

        auto impedanceP = sqrt(ui->inductanceP->value() / CdielectricP);
        ui->impedanceP->setValue(impedanceP);

        auto impedanceN = sqrt(ui->inductanceN->value() / CdielectricN);
        ui->impedanceN->setValue(impedanceN);

        ui->impedanceDiff->setValue(ui->impedanceP->value() + ui->impedanceN->value());

        // calculation complete
        ui->progress->setValue(100);
        ui->update->setEnabled(true);
        ui->abort->setEnabled(false);
        calculationStopped();
        ui->view->update();
    });

    auto calculationAborted = [=](){
        ui->progress->setValue(0);
        calculationStopped();
        ui->view->update();
    };

    connect(&laplace, &Laplace::calculationAborted, this, calculationAborted);

    auto scenarios = Scenario::createAll();
    for(auto s : scenarios) {
        auto action = new QAction(s->getName());
        ui->menuPredefined_Scenarios->addAction(action);
        connect(action, &QAction::triggered, this, [=](){
            s->show();
        });
        connect(s, &Scenario::scenarioCreated, this, [=](QPointF topLeft, QPointF bottomRight, ElementList *list){
            // set up new area
            ui->xleft->setValue(topLeft.x());
            ui->xright->setValue(bottomRight.x());
            ui->ytop->setValue(topLeft.y());
            ui->ybottom->setValue(bottomRight.y());
            // switch to the new elements
            ui->view->setElementList(list);
            ui->view->setSelectedElement(nullptr);
            delete this->list;
            this->list = list;
            ui->table->setModel(list);
            wireTableSelection();
            refreshGeometry();
        });
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

nlohmann::json MainWindow::toJSON()
{
    nlohmann::json j;
    // store simulation box imformation
    j["xleft"] = ui->xleft->value();
    j["xright"] = ui->xright->value();
    j["ytop"] = ui->ytop->value();
    j["ybottom"] = ui->ybottom->value();
    j["viewGrid"] = ui->gridsize->value();
    // store view settings
    j["showPotential"] = ui->showPotential->isChecked();
    j["showGrid"] = ui->showGrid->isChecked();
    j["snapToGrid"] = ui->snapGrid->isChecked();
    j["viewMode"] = ui->viewMode->currentText().toStdString();
    // store simulation parameters
    j["simulationGrid"] = ui->resolution->value();
    j["gaussDistance"] = ui->gaussDistance->value();
    j["tolerance"] = ui->tolerance->value();
    j["threads"] = ui->threads->value();
    j["borderIsGND"] = ui->borderIsGND->isChecked();
    // store parameters
    j["parameterList"] = params->toJSON();
    // store elements
    j["list"] = list->toJSON();
    return j;
}

void MainWindow::fromJSON(nlohmann::json j)
{
    // load simulation box information
    ui->xleft->setValue(j.value("xleft", ui->xleft->value()));
    ui->xright->setValue(j.value("xright", ui->xright->value()));
    ui->ytop->setValue(j.value("ytop", ui->ytop->value()));
    ui->ybottom->setValue(j.value("ybottom", ui->ybottom->value()));
    ui->gridsize->setValue(j.value("viewGrid", ui->gridsize->value()));
    // load view settings
    ui->showPotential->setChecked(j.value("showPotential", ui->showPotential->isChecked()));
    ui->showGrid->setChecked(j.value("showGrid", ui->showGrid->isChecked()));
    ui->snapGrid->setChecked(j.value("snapToGrid", ui->snapGrid->isChecked()));
    ui->viewMode->setCurrentText(QString::fromStdString(j.value("viewMode", ui->viewMode->currentText().toStdString())));
    // load simulation parameters
    ui->resolution->setValue(j.value("simulationGrid", ui->resolution->value()));
    ui->gaussDistance->setValue(j.value("gaussDistance", ui->gaussDistance->value()));
    ui->tolerance->setValue(j.value("tolerance", ui->tolerance->value()));
    ui->threads->setValue(j.value("threads", ui->threads->value()));
    ui->borderIsGND->setChecked(j.value("borderIsGND", ui->borderIsGND->isChecked()));
    // load parameters before elements so their symbols are available
    if(j.contains("parameterList")) {
        params->fromJSON(j["parameterList"]);
    }
    // load elements
    if(j.contains("list")) {
        list->fromJSON(j["list"]);
    }
    // resolve element vertices against the loaded parameters
    list->reevaluateAll(params->symbols());
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

void MainWindow::startCalculation()
{
    ui->progress->setValue(0);
    ui->update->setEnabled(false);
    ui->abort->setEnabled(true);
    ui->view->setEnabled(false);
    ui->table->setEnabled(false);
    ui->gridsize->setEnabled(false);
    ui->xleft->setEnabled(false);
    ui->xright->setEnabled(false);
    ui->ytop->setEnabled(false);
    ui->ybottom->setEnabled(false);
    ui->resolution->setEnabled(false);
    ui->gaussDistance->setEnabled(false);
    ui->threads->setEnabled(false);
    ui->tolerance->setEnabled(false);
    ui->borderIsGND->setEnabled(false);
    ui->add->setEnabled(false);
    ui->remove->setEnabled(false);
    ui->editPoints->setEnabled(false);
    ui->paramTable->setEnabled(false);
    ui->paramAdd->setEnabled(false);
    ui->paramRemove->setEnabled(false);

    // start the calculations
    ui->status->clear();
    ui->capacitanceP->setValue(std::numeric_limits<double>::quiet_NaN());
    ui->inductanceP->setValue(std::numeric_limits<double>::quiet_NaN());
    ui->impedanceP->setValue(std::numeric_limits<double>::quiet_NaN());
    ui->capacitanceN->setValue(std::numeric_limits<double>::quiet_NaN());
    ui->inductanceN->setValue(std::numeric_limits<double>::quiet_NaN());
    ui->impedanceN->setValue(std::numeric_limits<double>::quiet_NaN());
    ui->impedanceDiff->setValue(std::numeric_limits<double>::quiet_NaN());

    laplace.invalidateResult();
    ui->view->update();
    // TODO sanity check elements

    // check for self-intersecting polygons
    for(auto e : list->getElements()) {
        if(Polygon::selfIntersects(e->getVertices())) {
            error("Element \""+e->getName()+"\" self intersects, this is not supported");
            calculationStopped();
            return;
        }
    }
    // check for short circuits between RF and GND
    for(auto e1 : list->getElements()) {
        if(e1->getType() != Element::Type::GND) {
            continue;
        }
        for(auto e2 : list->getElements()) {
            if(e2->getType() != Element::Type::TracePos && e2->getType() != Element::Type::TraceNeg) {
                continue;
            }
            // check for overlap
            if(QPolygonF(e1->getVertices()).intersects(QPolygonF(e2->getVertices()))) {
                error("Short circuit between RF \""+e2->getName()+"\" and GND \""+e1->getName()+"\"");
                calculationStopped();
                return;
            }
        }
    }
    // check for overlapping/touching RF elements
    for(unsigned int i=0;i<list->getElements().size();i++) {
        auto e1 = list->getElements()[i];
        if(e1->getType() != Element::Type::TracePos && e1->getType() != Element::Type::TraceNeg) {
            continue;
        }
        for(unsigned int j=i+1;j<list->getElements().size();j++) {
            auto e2 = list->getElements()[j];
            if(e2->getType() != Element::Type::TracePos && e2->getType() != Element::Type::TraceNeg) {
                continue;
            }
            // check for overlap
            if(QPolygonF(e1->getVertices()).intersects(QPolygonF(e2->getVertices()))) {
                error("Traces \""+e2->getName()+"\" and \""+e1->getName()+"\" touch/overlap, this is not supported");
                calculationStopped();
                return;
            }
        }
    }
    // check and warn about overlapping dielectrics
    for(unsigned int i=0;i<list->getElements().size();i++) {
        auto e1 = list->getElements()[i];
        if(e1->getType() != Element::Type::Dielectric) {
            continue;
        }
        for(unsigned int j=i+1;j<list->getElements().size();j++) {
            auto e2 = list->getElements()[j];
            if(e2->getType() != Element::Type::Dielectric) {
                continue;
            }
            // check for overlap
            auto P1 = QPolygonF(e1->getVertices());
            auto P2 = QPolygonF(e2->getVertices());
            if(P1.intersects(P2)) {
                // check if this is actually an overlap or just touching
                auto intersect = P1.intersected(P2);
                // calculate area of intersection
                double area = 0;
                for(unsigned int k=0;k<intersect.size()-1;k++) {
                    auto s1 = intersect[k];
                    auto s2 = intersect[(k+1) % intersect.size()];
                    area += s1.x() * s2.y() - s2.x() * s1.y();
                }
                area = abs(area / 2);
                if(area > 0) {
                    warning("Dielectric \""+e1->getName()+"\" and \""+e2->getName()+"\" overlap, \""+e1->getName()+"\" will be used for overlapping area");
                }
            }
        }
    }

    connect(&laplace, &Laplace::percentage, this, [=](int percent){
        constexpr int minPercent = 0;
        constexpr int maxPercent = 99;
        ui->progress->setValue(percent * (maxPercent-minPercent) / 100 + minPercent);
    });
    connect(ui->abort, &QPushButton::clicked, &laplace, &Laplace::abortCalculation);

    // Start the dielectric laplace calculation
    laplace.setArea(ui->view->getTopLeft(), ui->view->getBottomRight());
    laplace.setGrid(ui->resolution->value());
    laplace.setThreads(ui->threads->value());
    laplace.setThreshold(ui->tolerance->value());
    laplace.setGroundedBorders(ui->borderIsGND->isChecked());
    laplace.startCalculation(list);
    ui->view->update();
}

void MainWindow::calculationStopped()
{
    ui->update->setEnabled(true);
    ui->abort->setEnabled(false);
    ui->view->setEnabled(true);
    ui->table->setEnabled(true);
    ui->gridsize->setEnabled(true);
    ui->xleft->setEnabled(true);
    ui->xright->setEnabled(true);
    ui->ytop->setEnabled(true);
    ui->ybottom->setEnabled(true);
    ui->resolution->setEnabled(true);
    ui->gaussDistance->setEnabled(true);
    ui->threads->setEnabled(true);
    ui->tolerance->setEnabled(true);
    ui->borderIsGND->setEnabled(true);
    ui->add->setEnabled(true);
    ui->remove->setEnabled(true);
    ui->editPoints->setEnabled(true);
    ui->paramTable->setEnabled(true);
    ui->paramAdd->setEnabled(true);
    ui->paramRemove->setEnabled(true);
}

void MainWindow::wireTableSelection()
{
    connect(ui->table->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            [=](const QModelIndex &current, const QModelIndex &){
        Element *e = nullptr;
        if(current.isValid() && current.row() < list->getElements().size()) {
            e = list->elementAt(current.row());
        }
        ui->view->setSelectedElement(e);
    });
}

void MainWindow::refreshGeometry()
{
    // recompute all element vertices from the current parameter values
    list->reevaluateAll(params->symbols());
    // the geometry changed, any previous field solution is no longer valid
    if(laplace.isResultReady()) {
        laplace.invalidateResult();
    }
    ui->view->update();
}

