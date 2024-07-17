#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "elementlist.h"
#include "laplace/laplace.h"
#include "gauss/gauss.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void info(QString info);
    void warning(QString warning);
    void error(QString error);

private:
    static constexpr double e0 = 8.8541878188e-12;
    Ui::MainWindow *ui;
    ElementList *list;
    Laplace laplace, laplaceAir;
    Gauss gauss;
    double Cdielectric;
};
#endif // MAINWINDOW_H
