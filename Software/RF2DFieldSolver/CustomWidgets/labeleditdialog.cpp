#include "labeleditdialog.h"
#include "ui_labeleditdialog.h"

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

enum Columns { ColName = 0, ColXExpr = 1, ColYExpr = 2, ColXValue = 3, ColYValue = 4 };

// Human-readable name for each anchor point of a label type.
static QStringList pointNames(Label::Type type)
{
    switch(type) {
    case Label::Type::Text: return {"Anchor"};
    case Label::Type::Dimension: return {"Start", "End"};
    case Label::Type::Last: break;
    }
    return {};
}

LabelEditDialog::LabelEditDialog(Label *label, const QMap<QString, double> &symbols, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::LabelEditDialog),
      label(label),
      symbols(symbols),
      updating(false)
{
    ui->setupUi(this);

    ui->table->setColumnCount(5);
    ui->table->setHorizontalHeaderLabels({"Point", "X expression", "Y expression", "X", "Y"});
    ui->table->horizontalHeader()->setSectionResizeMode(ColName, QHeaderView::ResizeToContents);
    ui->table->horizontalHeader()->setSectionResizeMode(ColXExpr, QHeaderView::Stretch);
    ui->table->horizontalHeader()->setSectionResizeMode(ColYExpr, QHeaderView::Stretch);
    ui->table->horizontalHeader()->setSectionResizeMode(ColXValue, QHeaderView::ResizeToContents);
    ui->table->horizontalHeader()->setSectionResizeMode(ColYValue, QHeaderView::ResizeToContents);

    for(auto t : Label::getTypes()) {
        ui->type->addItem(Label::TypeToString(t));
    }

    if(label) {
        ui->type->setCurrentIndex((int) label->getType());
        ui->labelText->setText(label->getText());
        rebuildRows(label->getType());
        // load existing expressions
        updating = true;
        for(int i=0;i<label->pointCount() && i<ui->table->rowCount();i++) {
            auto e = label->getPointExpr(i);
            ui->table->item(i, ColXExpr)->setText(e.first);
            ui->table->item(i, ColYExpr)->setText(e.second);
        }
        updating = false;
    } else {
        rebuildRows(Label::Type::Text);
    }
    updateAllPreviews();

    connect(ui->type, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int){
        rebuildRows(Label::TypeFromString(ui->type->currentText()));
        updateAllPreviews();
    });

    connect(ui->table, &QTableWidget::itemChanged, this, [=](QTableWidgetItem *item){
        if(updating) {
            return;
        }
        updatePreview(item->row());
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [=](){
        commit();
        accept();
    });
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

LabelEditDialog::~LabelEditDialog()
{
    delete ui;
}

void LabelEditDialog::rebuildRows(Label::Type type)
{
    // preserve whatever expressions are currently entered, by row index
    QList<QPair<QString, QString>> existing;
    for(int i=0;i<ui->table->rowCount();i++) {
        auto x = ui->table->item(i, ColXExpr);
        auto y = ui->table->item(i, ColYExpr);
        existing.append({x ? x->text() : "0", y ? y->text() : "0"});
    }

    updating = true;
    ui->table->setRowCount(0);
    auto names = pointNames(type);
    for(int i=0;i<names.size();i++) {
        ui->table->insertRow(i);
        auto nameItem = new QTableWidgetItem(names[i]);
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        ui->table->setItem(i, ColName, nameItem);
        QString x = i < existing.size() ? existing[i].first : "0";
        QString y = i < existing.size() ? existing[i].second : "0";
        ui->table->setItem(i, ColXExpr, new QTableWidgetItem(x));
        ui->table->setItem(i, ColYExpr, new QTableWidgetItem(y));
        for(int col : {ColXValue, ColYValue}) {
            auto item = new QTableWidgetItem();
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            ui->table->setItem(i, col, item);
        }
    }
    updating = false;
}

void LabelEditDialog::updatePreview(int row)
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

void LabelEditDialog::updateAllPreviews()
{
    for(int i=0;i<ui->table->rowCount();i++) {
        updatePreview(i);
    }
}

void LabelEditDialog::commit()
{
    if(!label) {
        return;
    }
    label->setText(ui->labelText->text());
    label->setType(Label::TypeFromString(ui->type->currentText()));
    QList<QPair<QString, QString>> exprs;
    for(int i=0;i<ui->table->rowCount();i++) {
        auto xItem = ui->table->item(i, ColXExpr);
        auto yItem = ui->table->item(i, ColYExpr);
        QString x = xItem ? xItem->text() : QString("0");
        QString y = yItem ? yItem->text() : QString("0");
        exprs.append({x, y});
    }
    label->setPointExpressions(exprs, symbols);
}
