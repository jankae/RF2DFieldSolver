#include "pointseditdialog.h"
#include "ui_pointseditdialog.h"

#include "expression.h"
#include "unit.h"

#include <QTableWidgetItem>
#include <QHeaderView>
#include <QPushButton>
#include <QBrush>
#include <QColor>
#include <cmath>

// Coordinates are lengths; preview them in meters with SI prefixes.
static const QString PREVIEW_UNIT = "m";
static const QString PREVIEW_PREFIXES = "fpnum kMGTP";

enum Columns { ColXExpr = 0, ColYExpr = 1, ColXValue = 2, ColYValue = 3 };

PointsEditDialog::PointsEditDialog(Element *element, const QMap<QString, double> &symbols, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::PointsEditDialog),
      element(element),
      symbols(symbols),
      updating(false)
{
    ui->setupUi(this);
    if(element) {
        setWindowTitle("Edit Points - " + element->getName());
    }

    ui->table->setColumnCount(4);
    ui->table->setHorizontalHeaderLabels({"X expression", "Y expression", "X", "Y"});
    ui->table->horizontalHeader()->setSectionResizeMode(ColXExpr, QHeaderView::Stretch);
    ui->table->horizontalHeader()->setSectionResizeMode(ColYExpr, QHeaderView::Stretch);
    ui->table->horizontalHeader()->setSectionResizeMode(ColXValue, QHeaderView::ResizeToContents);
    ui->table->horizontalHeader()->setSectionResizeMode(ColYValue, QHeaderView::ResizeToContents);

    // load existing vertices
    updating = true;
    if(element) {
        for(int i=0;i<element->vertexCount();i++) {
            auto e = element->getVertexExpr(i);
            addRow(i, e.first, e.second);
        }
    }
    updating = false;
    updateAllPreviews();

    connect(ui->table, &QTableWidget::itemChanged, this, [=](QTableWidgetItem *item){
        if(updating) {
            return;
        }
        updatePreview(item->row());
    });

    connect(ui->add, &QPushButton::clicked, this, [=](){
        int row = ui->table->rowCount();
        addRow(row, "0", "0");
        updatePreview(row);
        ui->table->setCurrentCell(row, ColXExpr);
    });
    connect(ui->insert, &QPushButton::clicked, this, [=](){
        int row = ui->table->currentRow();
        if(row < 0) {
            row = ui->table->rowCount();
        }
        addRow(row, "0", "0");
        updatePreview(row);
        ui->table->setCurrentCell(row, ColXExpr);
    });
    connect(ui->remove, &QPushButton::clicked, this, [=](){
        int row = ui->table->currentRow();
        if(row >= 0) {
            ui->table->removeRow(row);
        }
    });
    connect(ui->moveUp, &QPushButton::clicked, this, [=](){
        int row = ui->table->currentRow();
        swapRows(row, row - 1);
    });
    connect(ui->moveDown, &QPushButton::clicked, this, [=](){
        int row = ui->table->currentRow();
        swapRows(row, row + 1);
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [=](){
        commit();
        accept();
    });
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

PointsEditDialog::~PointsEditDialog()
{
    delete ui;
}

void PointsEditDialog::addRow(int at, const QString &xExpr, const QString &yExpr)
{
    bool prev = updating;
    updating = true;
    ui->table->insertRow(at);
    ui->table->setItem(at, ColXExpr, new QTableWidgetItem(xExpr));
    ui->table->setItem(at, ColYExpr, new QTableWidgetItem(yExpr));
    for(int col : {ColXValue, ColYValue}) {
        auto item = new QTableWidgetItem();
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        ui->table->setItem(at, col, item);
    }
    updating = prev;
}

void PointsEditDialog::swapRows(int a, int b)
{
    if(a < 0 || b < 0 || a >= ui->table->rowCount() || b >= ui->table->rowCount()) {
        return;
    }
    updating = true;
    for(int col : {ColXExpr, ColYExpr}) {
        auto ta = ui->table->item(a, col)->text();
        auto tb = ui->table->item(b, col)->text();
        ui->table->item(a, col)->setText(tb);
        ui->table->item(b, col)->setText(ta);
    }
    updating = false;
    updatePreview(a);
    updatePreview(b);
    ui->table->setCurrentCell(b, ui->table->currentColumn());
}

void PointsEditDialog::updatePreview(int row)
{
    if(row < 0 || row >= ui->table->rowCount()) {
        return;
    }
    struct { int exprCol; int valueCol; } pairs[] = {
        { ColXExpr, ColXValue },
        { ColYExpr, ColYValue },
    };
    updating = true;
    for(auto &p : pairs) {
        auto exprItem = ui->table->item(row, p.exprCol);
        auto valueItem = ui->table->item(row, p.valueCol);
        if(!exprItem || !valueItem) {
            continue;
        }
        QString err;
        double v = Expression::evaluate(exprItem->text(), symbols, &err);
        if(std::isnan(v)) {
            valueItem->setText("error");
            valueItem->setToolTip(err);
            valueItem->setForeground(QBrush(QColor(200, 0, 0)));
        } else {
            valueItem->setText(Unit::ToString(v, PREVIEW_UNIT, PREVIEW_PREFIXES, 4));
            valueItem->setToolTip(QString());
            valueItem->setForeground(QBrush());
        }
    }
    updating = false;
}

void PointsEditDialog::updateAllPreviews()
{
    for(int i=0;i<ui->table->rowCount();i++) {
        updatePreview(i);
    }
}

void PointsEditDialog::commit()
{
    if(!element) {
        return;
    }
    QList<QPair<QString, QString>> exprs;
    for(int i=0;i<ui->table->rowCount();i++) {
        auto xItem = ui->table->item(i, ColXExpr);
        auto yItem = ui->table->item(i, ColYExpr);
        QString x = xItem ? xItem->text() : QString("0");
        QString y = yItem ? yItem->text() : QString("0");
        exprs.append({x, y});
    }
    element->setVertexExpressions(exprs, symbols);
}
