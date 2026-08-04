#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QScrollBar>

#include <QDebug>
#include <QVector>
#include <QItemSelectionModel>

#include "polygon.h"
#include "expression.h"

#include "CustomWidgets/pointseditdialog.h"
#include "CustomWidgets/labeleditdialog.h"
#include "CustomWidgets/examplebrowserdialog.h"
#include "CustomWidgets/informationbox.h"

#include <QMenuBar>
#include <QActionGroup>
#include <cmath>

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
    ui->resolution->setValue(4e-6);

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

    // The area bounds may be parameterised (auto-size). A manual edit of a field
    // detaches that bound from its expression; programmatic updates (from loading
    // or re-evaluating parameters) are guarded by updatingArea.
    connect(ui->xleft, &SIUnitEdit::valueChanged, this, [=](){ if(!updatingArea) xleftExpr.clear(); });
    connect(ui->xright, &SIUnitEdit::valueChanged, this, [=](){ if(!updatingArea) xrightExpr.clear(); });
    connect(ui->ytop, &SIUnitEdit::valueChanged, this, [=](){ if(!updatingArea) ytopExpr.clear(); });
    connect(ui->ybottom, &SIUnitEdit::valueChanged, this, [=](){ if(!updatingArea) ybottomExpr.clear(); });

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

    // duplicate the selected element (handy for e.g. GND on either side of a trace)
    connect(ui->duplicate, &QPushButton::clicked, this, [=](){
        auto row = ui->table->currentIndex().row();
        if(row < 0 || row >= list->getElements().size()) {
            return;
        }
        auto src = list->elementAt(row);
        auto copy = new Element(src->getType());
        copy->setName(src->getName() + " copy");
        copy->setEpsilonR(src->getEpsilonR());
        QList<QPair<QString, QString>> exprs;
        for(int i=0;i<src->vertexCount();i++) {
            exprs.append(src->getVertexExpr(i));
        }
        copy->setVertexExpressions(exprs, params->symbols());
        list->addElement(copy);
        int newRow = list->getElements().indexOf(copy);
        if(newRow >= 0) {
            ui->table->selectRow(newRow);
        }
        refreshGeometry();
    });

    // labels (text annotations and dimension arrows drawn in the view)
    labels = new LabelList();
    ui->labelsTable->setModel(labels);
    ui->labelsTable->setItemDelegateForColumn((int) LabelList::Column::Type, new LabelTypeDelegate());
    ui->view->setLabelList(labels);
    // repaint the view whenever a label is added/removed/edited inline
    connect(labels, &QAbstractItemModel::dataChanged, this, [=](){ ui->view->update(); });
    connect(labels, &QAbstractItemModel::rowsInserted, this, [=](){ ui->view->update(); });
    connect(labels, &QAbstractItemModel::rowsRemoved, this, [=](){ ui->view->update(); });

    // The labels panel sits below the elements table (both share one splitter
    // pane via a plain vertical layout). It is collapsed by default: unchecking
    // it hides its contents so only the "Labels" title bar remains, keeping it
    // out of the way. Its size policy (set in the .ui) stops it from stretching,
    // so the elements table keeps the space.
    connect(ui->labelsBox, &QGroupBox::toggled, ui->labelsContent, &QWidget::setVisible);
    ui->labelsContent->setVisible(ui->labelsBox->isChecked());

    // add labels (Text or Dimension) via a small dropdown menu
    auto labelAddMenu = new QMenu();
    auto addText = new QAction("Text");
    connect(addText, &QAction::triggered, this, [=](){
        auto l = new Label(Label::Type::Text);
        labels->addLabel(l);
        ui->labelsTable->selectRow(labels->getLabels().size() - 1);
        refreshGeometry();
    });
    labelAddMenu->addAction(addText);
    auto addDimension = new QAction("Dimension");
    connect(addDimension, &QAction::triggered, this, [=](){
        auto l = new Label(Label::Type::Dimension);
        labels->addLabel(l);
        ui->labelsTable->selectRow(labels->getLabels().size() - 1);
        refreshGeometry();
    });
    labelAddMenu->addAction(addDimension);
    ui->labelAdd->setMenu(labelAddMenu);

    connect(ui->labelDuplicate, &QPushButton::clicked, this, [=](){
        auto row = ui->labelsTable->currentIndex().row();
        if(row < 0 || row >= labels->getLabels().size()) {
            return;
        }
        auto src = labels->labelAt(row);
        auto copy = new Label(src->getType());
        copy->setText(src->getText());
        QList<QPair<QString, QString>> exprs;
        for(int i=0;i<src->pointCount();i++) {
            exprs.append(src->getPointExpr(i));
        }
        copy->setPointExpressions(exprs, params->symbols());
        labels->addLabel(copy);
        int newRow = labels->getLabels().indexOf(copy);
        if(newRow >= 0) {
            ui->labelsTable->selectRow(newRow);
        }
        refreshGeometry();
    });

    connect(ui->labelRemove, &QPushButton::clicked, this, [=](){
        auto row = ui->labelsTable->currentIndex().row();
        if(row >= 0 && row < labels->getLabels().size()) {
            labels->removeLabel(row);
            ui->view->update();
        }
    });

    connect(ui->labelEdit, &QPushButton::clicked, this, [=](){
        auto row = ui->labelsTable->currentIndex().row();
        if(row < 0 || row >= labels->getLabels().size()) {
            return;
        }
        auto l = labels->labelAt(row);
        LabelEditDialog d(l, params->symbols(), this);
        if(d.exec() == QDialog::Accepted) {
            refreshGeometry();
        }
    });

    // View menu (label/contour toggles and label text size)
    setupViewMenu();

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

    // Examples: browse and load built-in parameterised example projects
    connect(ui->actionBrowseExamples, &QAction::triggered, this, [=](){
        ExampleBrowserDialog d(this);
        if(d.exec() != QDialog::Accepted) {
            return;
        }
        auto path = d.selectedResourcePath();
        if(path.isEmpty()) {
            return;
        }
        if(!InformationBox::AskQuestion("Load example",
                "Discard the current project and load this example?", true)) {
            return;
        }
        // examples load in place: fromJSON refreshes the existing models
        ui->view->setSelectedElement(nullptr);
        if(openFromResource(path)) {
            refreshGeometry();
            ui->view->update();
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

nlohmann::json MainWindow::toJSON()
{
    nlohmann::json j;
    // store simulation box imformation (resolved values, backward compatible)
    j["xleft"] = ui->xleft->value();
    j["xright"] = ui->xright->value();
    j["ytop"] = ui->ytop->value();
    j["ybottom"] = ui->ybottom->value();
    // and the optional expressions backing them (auto-size)
    if(!xleftExpr.isEmpty())   j["xleftExpr"]   = xleftExpr.toStdString();
    if(!xrightExpr.isEmpty())  j["xrightExpr"]  = xrightExpr.toStdString();
    if(!ytopExpr.isEmpty())    j["ytopExpr"]    = ytopExpr.toStdString();
    if(!ybottomExpr.isEmpty()) j["ybottomExpr"] = ybottomExpr.toStdString();
    j["viewGrid"] = ui->gridsize->value();
    // store view settings
    j["showPotential"] = ui->showPotential->isChecked();
    j["showGrid"] = ui->showGrid->isChecked();
    j["snapToGrid"] = ui->snapGrid->isChecked();
    j["viewMode"] = ui->viewMode->currentText().toStdString();
    j["showLabels"] = ui->view->getShowLabels();
    j["fillContours"] = ui->view->getFillContours();
    j["labelTextSize"] = ui->view->getLabelTextSize();
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
    // store labels
    j["labels"] = labels->toJSON();
    return j;
}

void MainWindow::fromJSON(nlohmann::json j)
{
    // load simulation box information. Guard so loading the numeric values does
    // not clear the expressions we are about to load.
    updatingArea = true;
    ui->xleft->setValue(j.value("xleft", ui->xleft->value()));
    ui->xright->setValue(j.value("xright", ui->xright->value()));
    ui->ytop->setValue(j.value("ytop", ui->ytop->value()));
    ui->ybottom->setValue(j.value("ybottom", ui->ybottom->value()));
    // optional area expressions (absent ⇒ fixed numeric area, backward compatible)
    xleftExpr   = QString::fromStdString(j.value("xleftExpr", std::string()));
    xrightExpr  = QString::fromStdString(j.value("xrightExpr", std::string()));
    ytopExpr    = QString::fromStdString(j.value("ytopExpr", std::string()));
    ybottomExpr = QString::fromStdString(j.value("ybottomExpr", std::string()));
    updatingArea = false;
    ui->gridsize->setValue(j.value("viewGrid", ui->gridsize->value()));
    // load view settings
    ui->showPotential->setChecked(j.value("showPotential", ui->showPotential->isChecked()));
    ui->showGrid->setChecked(j.value("showGrid", ui->showGrid->isChecked()));
    ui->snapGrid->setChecked(j.value("snapToGrid", ui->snapGrid->isChecked()));
    ui->viewMode->setCurrentText(QString::fromStdString(j.value("viewMode", ui->viewMode->currentText().toStdString())));
    // label/contour view settings (checking the menu actions also updates the view)
    actShowLabels->setChecked(j.value("showLabels", ui->view->getShowLabels()));
    actFillContours->setChecked(j.value("fillContours", ui->view->getFillContours()));
    applyLabelTextSize(j.value("labelTextSize", ui->view->getLabelTextSize()));
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
    // load labels (older files without this key simply have no labels)
    if(j.contains("labels")) {
        labels->fromJSON(j["labels"]);
    }
    // resolve element vertices and label points against the loaded parameters
    list->reevaluateAll(params->symbols());
    labels->reevaluateAll(params->symbols());
    // resolve the (optional) parameterised area bounds
    applyAreaExpressions();
    ui->view->update();
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
    ui->duplicate->setEnabled(false);
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
    ui->duplicate->setEnabled(true);
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

void MainWindow::setupViewMenu()
{
    auto viewMenu = new QMenu("View", this);
    // place View before the Predefined Scenarios menu for conventional ordering
    menuBar()->insertMenu(ui->menuPredefined_Scenarios->menuAction(), viewMenu);

    actShowLabels = viewMenu->addAction("Show Labels");
    actShowLabels->setCheckable(true);
    actShowLabels->setChecked(ui->view->getShowLabels());
    connect(actShowLabels, &QAction::toggled, this, [=](bool on){
        ui->view->setShowLabels(on);
    });

    actFillContours = viewMenu->addAction("Fill Contours");
    actFillContours->setCheckable(true);
    actFillContours->setChecked(ui->view->getFillContours());
    connect(actFillContours, &QAction::toggled, this, [=](bool on){
        ui->view->setFillContours(on);
    });

    auto sizeMenu = viewMenu->addMenu("Label Text Size");
    auto sizeGroup = new QActionGroup(this);
    sizeGroup->setExclusive(true);
    struct { const char *name; int px; } sizes[] = {
        {"Small", 10}, {"Medium", 14}, {"Large", 20}, {"Extra Large", 28},
    };
    for(auto &s : sizes) {
        auto a = sizeMenu->addAction(s.name);
        a->setCheckable(true);
        sizeGroup->addAction(a);
        labelSizeActions.insert(s.px, a);
        int px = s.px;
        connect(a, &QAction::triggered, this, [=](){
            ui->view->setLabelTextSize(px);
        });
    }
    // reflect the view's current size in the menu
    applyLabelTextSize(ui->view->getLabelTextSize());
}

void MainWindow::applyLabelTextSize(int pixels)
{
    ui->view->setLabelTextSize(pixels);
    if(labelSizeActions.contains(pixels)) {
        labelSizeActions[pixels]->setChecked(true);
    }
}

void MainWindow::applyAreaExpressions()
{
    struct { QString *expr; SIUnitEdit *edit; } bounds[] = {
        { &xleftExpr, ui->xleft }, { &xrightExpr, ui->xright },
        { &ytopExpr, ui->ytop }, { &ybottomExpr, ui->ybottom },
    };
    // guard so setting the resolved value does not clear the expression
    updatingArea = true;
    for(auto &b : bounds) {
        if(b.expr->isEmpty()) {
            continue;
        }
        double v = Expression::evaluate(*b.expr, params->symbols(), nullptr);
        if(!std::isnan(v)) {
            b.edit->setValue(v);
        }
    }
    updatingArea = false;
}

void MainWindow::refreshGeometry()
{
    // recompute all element vertices from the current parameter values
    list->reevaluateAll(params->symbols());
    // labels share the same parameters, keep their resolved points in sync
    labels->reevaluateAll(params->symbols());
    // re-size the (optionally) parameterised simulation area
    applyAreaExpressions();
    // the geometry changed, any previous field solution is no longer valid
    if(laplace.isResultReady()) {
        laplace.invalidateResult();
    }
    ui->view->update();
}

