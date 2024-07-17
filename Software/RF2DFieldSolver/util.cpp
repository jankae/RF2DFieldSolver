#include "util.h"

#include <QVector2D>

double Util::distanceToLine(QPointF point, QPointF l1, QPointF l2, QPointF *closestLinePoint, double *pointRatio)
{
    auto M = l2 - l1;
    auto t0 = QPointF::dotProduct(M, point - l1) / QPointF::dotProduct(M, M);
    QPointF closestPoint;
    QVector2D orthVect;
    if (t0 <= 0) {
        orthVect = QVector2D(point - l1);
        closestPoint = l1;
        t0 = 0;
    } else if(t0 >= 1) {
        orthVect = QVector2D(point - l2);
        closestPoint = l2;
        t0 = 1;
    } else {
        auto intersect = l1 + t0 * M;
        orthVect = QVector2D(point - intersect);
        closestPoint = intersect;
    }
    if(closestLinePoint) {
        *closestLinePoint = closestPoint;
    }
    if(pointRatio) {
        *pointRatio = t0;
    }
    return orthVect.length();
}

QColor Util::getIntensityGradeColor(double intensity)
{
    if(intensity < 0.0) {
        return Qt::black;
    } else if(intensity > 1.0) {
        return Qt::white;
    } else if(intensity >= 0.0 && intensity <= 1.0) {
        return QColor::fromHsv(Util::Scale<double>(intensity, 0.0, 1.0, 240, 0), 255, 255);
    } else {
        return Qt::black;
    }
}
