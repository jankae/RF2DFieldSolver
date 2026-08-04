#ifndef LABELEDITDIALOG_H
#define LABELEDITDIALOG_H

#include <QDialog>
#include <QMap>
#include "label.h"

namespace Ui {
class LabelEditDialog;
}

// Modal editor for a single label. The type and text are edited directly; the
// anchor point(s) are entered as expressions (number or equation over the
// parameters) with a live evaluated preview. On accept the values are written
// back to the label.
class LabelEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LabelEditDialog(Label *label, const QMap<QString, double> &symbols, QWidget *parent = nullptr);
    ~LabelEditDialog();

private:
    // Rebuilds the coordinate rows to match the currently selected type,
    // preserving any expressions already entered.
    void rebuildRows(Label::Type type);
    void updatePreview(int row);
    void updateAllPreviews();
    void commit();

    Ui::LabelEditDialog *ui;
    Label *label;
    QMap<QString, double> symbols;
    bool updating;   // guards itemChanged while we mutate the table ourselves
};

#endif // LABELEDITDIALOG_H
