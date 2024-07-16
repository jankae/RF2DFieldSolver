#ifndef UTILH_H
#define UTILH_H

#include <QPoint>

namespace Util {
    double distanceToLine(QPointF point, QPointF l1, QPointF l2, QPointF *closestLinePoint = nullptr, double *pointRatio = nullptr);
}

#endif // UTILH_H
