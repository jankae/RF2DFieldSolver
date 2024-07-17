#ifndef GAUSS_H
#define GAUSS_H

#include <QObject>

#include "laplace/laplace.h"
#include "elementlist.h"

class Gauss : public QObject
{
    Q_OBJECT
public:
    explicit Gauss(QObject *parent = nullptr);

    static double getCharge(Laplace *laplace, ElementList *list, Element *e, double gridSize);

signals:
    void info(QString info);
    void warning(QString warning);
    void error(QString error);
};

#endif // GAUSS_H
