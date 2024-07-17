#include "laplace.h"

#include <QPolygonF>

Laplace::Laplace(QObject *parent)
    : QObject{parent}
{
    calculationRunning = false;
    resultReady = false;
    list = nullptr;
    grid = 1e-5;
    threads = 1;
    threshold = 1e-6;
    lattice = nullptr;
    groundedBorders = true;
    ignoreDielectric = false;
}

void Laplace::setArea(const QPointF &topLeft, const QPointF &bottomRight)
{
    if(calculationRunning) {
        return;
    }
    this->topLeft = topLeft;
    this->bottomRight = bottomRight;
}

void Laplace::setGrid(double grid)
{
    if(calculationRunning) {
        return;
    }
    if(grid > 0) {
        this->grid = grid;
    }
}

void Laplace::setThreads(int threads)
{
    if(calculationRunning) {
        return;
    }
    if(threads > 0) {
        this->threads = threads;
    }
}

void Laplace::setThreshold(double threshold)
{
    if(calculationRunning) {
        return;
    }
    if (threshold > 0) {
        this->threshold = threshold;
    }
}

void Laplace::setGroundedBorders(bool gnd)
{
    if(calculationRunning) {
        return;
    }
    groundedBorders = gnd;
}

void Laplace::setIgnoreDielectric(bool ignore)
{
    if(calculationRunning) {
        return;
    }
    ignoreDielectric = ignore;
}

bool Laplace::startCalculation(ElementList *list)
{
    if(calculationRunning) {
        return false;
    }
    calculationRunning = true;
    resultReady = false;
    emit info("Laplace calculation starting");
    if(lattice) {
        delete lattice;
        lattice = nullptr;
    }
    this->list = list;

    // start the calculation thread
    auto err = pthread_create(&thread, nullptr, calcThreadTrampoline, this);
    if(err) {
        emit error("Failed to start laplace thread");
        return false;
    }
    emit info("Laplace thread started");
    return true;
}

void Laplace::abortCalculation()
{
    if(!calculationRunning) {
        return;
    }
    calculationRunning = false;
    resultReady = false;
    // TODO stop threads
}

double Laplace::getPotential(const QPointF &p)
{
    if(!resultReady) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    auto pos = coordToRect(p);
    // convert to integers and shift by the added outside boundary of NaNs
    int index_x = round(pos.x) + 1;
    int index_y = round(pos.y) + 1;
    if(index_x < 0 || index_x >= (int) lattice->dim.x || index_y < 0 || index_y >= (int) lattice->dim.y) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    auto c = &lattice->cells[index_x+index_y*lattice->dim.x];
    return c->value;
}

QLineF Laplace::getGradient(const QPointF &p)
{
    QLineF ret = QLineF(p, p);
    if(!resultReady) {
        return ret;
    }
    auto pos = coordToRect(p);
    // convert to integers and shift by the added outside boundary of NaNs
    int index_x = floor(pos.x) + 1;
    int index_y = floor(pos.y) + 1;

    if(index_x < 0 || index_x + 1 >= (int) lattice->dim.x || index_y < 0 || index_y + 1>= (int) lattice->dim.y) {
        return ret;
    }
    // calculate gradient
    auto c_floor = &lattice->cells[index_x+index_y*lattice->dim.x];
    auto c_x = &lattice->cells[index_x+1+index_y*lattice->dim.x];
    auto c_y = &lattice->cells[index_x+(index_y+1)*lattice->dim.x];
    auto grad_x = c_x->value - c_floor->value;
    auto grad_y = c_y->value - c_floor->value;
    ret.setP2(p + QPointF(grad_x, grad_y));
    return ret;
}

void Laplace::invalidateResult()
{
    resultReady = false;
}

QPointF Laplace::coordFromRect(rect *pos)
{
    QPointF ret;
    ret.rx() = pos->x * grid + topLeft.x();
    ret.ry() = pos->y * grid + bottomRight.y();
    return ret;
}

struct rect Laplace::coordToRect(const QPointF &pos)
{
    struct rect ret;
    ret.x = (pos.x() - topLeft.x()) / grid;
    ret.y = (pos.y() - bottomRight.y()) / grid;
    return ret;
}

bound *Laplace::boundaryTrampoline(bound *bound, rect *pos)
{
    auto coord = coordFromRect(pos);
//    qDebug() << pos->x << pos->y;
//    qDebug() << coord;
    bound->value = 0;
    bound->cond = NONE;

    bool isBorder = qFuzzyCompare(coord.x(), topLeft.x()) || qFuzzyCompare(coord.x(), bottomRight.x()) || qFuzzyCompare(coord.y(), topLeft.y()) || qFuzzyCompare(coord.y(), bottomRight.y());

    // handle borders
    if(groundedBorders && isBorder) {
        bound->value = 0;
        bound->cond = DIRICHLET;
        return bound;
    } else {
        // find the matching polygon
        for(auto e : list->getElements()) {
            QPolygonF poly = e->toPolygon();
            if(poly.containsPoint(coord, Qt::OddEvenFill)) {
//                qDebug() << "in polygon";
                // this polygon defines the boundary at these coordinates
                switch(e->getType()) {
                case Element::Type::GND:
                    bound->value = 0;
                    bound->cond = DIRICHLET;
                    return bound;
                case Element::Type::Trace:
                    bound->value = 1.0;
                    bound->cond = DIRICHLET;
                    return bound;
                case Element::Type::Dielectric:
                case Element::Type::Last:
                    return bound;
                }
            }
        }
    }
    return bound;
}

double Laplace::weight(rect *pos)
{
    if(ignoreDielectric) {
        return 1.0;
    }

    auto coord = coordFromRect(pos);
    return sqrt(list->getDielectricConstantAt(coord));
}

void* Laplace::calcThread()
{
    emit info("Creating lattice");
    struct rect size = {(bottomRight.x() - topLeft.x()) / grid, (topLeft.y() - bottomRight.y()) / grid};
    struct point dim = {(uint32_t) ((bottomRight.x() - topLeft.x()) / grid), (uint32_t) ((topLeft.y() - bottomRight.y()) / grid)};
    lattice = lattice_new(&size, &dim, &boundaryTrampoline, &weightTrampoline, this);
    if(lattice) {
        emit info("Lattice creation complete");
    } else {
        emit error("Lattice creation failed");
        return nullptr;
    }

    struct config conf = {(uint8_t) threads, 10, threshold};
    if(conf.threads > lattice->dim.y / 5) {
        conf.threads = lattice->dim.y / 5;
    }
    conf.distance = lattice->dim.y / threads;
    emit info("Starting calculation threads");
    auto it = lattice_compute_threaded(lattice, &conf);
    emit info("Laplace calculation complete, took "+QString::number(it)+" iterations");
    calculationRunning = false;
    resultReady = true;
    emit calculationDone();
    return nullptr;
}
