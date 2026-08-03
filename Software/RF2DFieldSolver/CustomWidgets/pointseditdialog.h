#ifndef POINTSEDITDIALOG_H
#define POINTSEDITDIALOG_H

#include <QDialog>
#include <QMap>
#include "element.h"

namespace Ui {
class PointsEditDialog;
}

// Modal editor for the point list of a single element. Each point's X and Y are
// entered as expressions (number or equation over the parameters) with a live
// evaluated preview. On accept the expressions are written back to the element.
class PointsEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PointsEditDialog(Element *element, const QMap<QString, double> &symbols, QWidget *parent = nullptr);
    ~PointsEditDialog();

private:
    void addRow(int at, const QString &xExpr, const QString &yExpr);
    void swapRows(int a, int b);
    void updatePreview(int row);
    void updateAllPreviews();
    void commit();

    Ui::PointsEditDialog *ui;
    Element *element;
    QMap<QString, double> symbols;
    bool updating;   // guards itemChanged while we mutate the table ourselves
};

#endif // POINTSEDITDIALOG_H
