#ifndef LAPLACE_H
#define LAPLACE_H

#include <QObject>
#include <QPointF>

#include <pthread.h>

#include "elementlist.h"
#include "lattice.h"

class Laplace : public QObject
{
    Q_OBJECT
public:
    explicit Laplace(QObject *parent = nullptr);

    void setArea(const QPointF &topLeft, const QPointF &bottomRight);
    void setGrid(double grid);
    void setThreads(int threads);
    void setThreshold(double threshold);
    void setGroundedBorders(bool gnd);
    void setIgnoreDielectric(bool ignore);

    bool startCalculation(ElementList *list);
    void abortCalculation();
    double getPotential(const QPointF &p);
    QLineF getGradient(const QPointF &p);
    bool isResultReady() {return resultReady;}
    void invalidateResult();

    double weight(rect *pos);

signals:
    void percentage(int percent);
    void calculationDone();
    void info(QString info);
    void warning(QString warning);
    void error(QString error);

private:
    QPointF coordFromRect(struct rect *pos);
    struct rect coordToRect(const QPointF &pos);
    bound* boundaryTrampoline(struct bound* bound, struct rect* pos);
    static struct bound* boundaryTrampoline(void *ptr, struct bound* bound, struct rect* pos) {
        return ((Laplace*)ptr)->boundaryTrampoline(bound, pos);
    }
    static double weightTrampoline(void *ptr, struct rect* pos) {
        return ((Laplace*)ptr)->weight(pos);
    }
    void* calcThread();
    static void* calcThreadTrampoline(void *ptr) {
        return ((Laplace*)ptr)->calcThread();
    }
    bool calculationRunning;
    bool resultReady;
    ElementList *list;
    QPointF topLeft, bottomRight;
    double grid;
    int threads;
    double threshold;
    bool groundedBorders;
    bool ignoreDielectric;
    struct lattice *lattice;

    pthread_t thread;
};

#endif // LAPLACE_H
