#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "elementlist.h"
#include "labellist.h"
#include "parameterlist.h"
#include "laplace/laplace.h"
#include "gauss/gauss.h"
#include "savable.h"

class QAction;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow, public Savable
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

private slots:
    void info(QString info);
    void warning(QString warning);
    void error(QString error);

private:
    static constexpr double e0 = 8.8541878188e-12;
    void startCalculation();
    void calculationStopped();
    // Re-evaluates all element vertices from the current parameter values and
    // repaints the view. Called whenever a parameter or point changes.
    void refreshGeometry();
    // Connects the elements table selection to the view highlight. Must be
    // called again whenever the table's model (and thus its selection model)
    // is replaced.
    void wireTableSelection();
    // Builds the View menu (label/contour toggles and label text size) and wires
    // it to the view.
    void setupViewMenu();
    // Applies a label text size to the view and checks the matching menu entry.
    void applyLabelTextSize(int pixels);
    // Evaluates the (optional) area-bound expressions against the current
    // parameters and pushes the results into the area fields.
    void applyAreaExpressions();
    Ui::MainWindow *ui;
    ElementList *list;
    LabelList *labels;
    ParameterList *params;
    Laplace laplace;
    Gauss gauss;
    // View menu actions kept so their state can be restored from a project file.
    QAction *actShowLabels;
    QAction *actFillContours;
    QMap<int, QAction*> labelSizeActions;
    // Optional expressions backing the simulation-area bounds (empty ⇒ the bound
    // is a fixed number). updatingArea guards programmatic field updates so they
    // are not mistaken for a manual edit that would clear the expression.
    QString xleftExpr, xrightExpr, ytopExpr, ybottomExpr;
    bool updatingArea = false;
};
#endif // MAINWINDOW_H
