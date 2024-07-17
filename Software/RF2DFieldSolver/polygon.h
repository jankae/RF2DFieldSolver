#ifndef POLYGON_H
#define POLYGON_H

#include <QPointF>
#include <QList>

namespace Polygon {

    bool selfIntersects(const QList<QPointF> &vertices);
    QList<QPointF> offset(const QList<QPointF> &vertices, double offset);
    bool isClockwise(const QList<QPointF> &vertices);

}

#endif // POLYGON_H
