#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "elementlist.h"
#include "laplace/laplace.h"
#include "gauss/gauss.h"
#include "savable.h"

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
    Ui::MainWindow *ui;
    ElementList *list;
    Laplace laplace;
    Gauss gauss;
};
#endif // MAINWINDOW_H
