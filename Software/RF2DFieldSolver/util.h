#ifndef UTILH_H
#define UTILH_H

#include <QPoint>
#include <QColor>

namespace Util {

    template<typename T> T Scale(T value, T from_low, T from_high, T to_low, T to_high, bool log_from = false, bool log_to = false) {
        T normalized;
        if(log_from) {
            normalized = log10(value / from_low) / log10(from_high / from_low);
        } else {
            normalized = (value - from_low) / (from_high - from_low);
        }
        if(log_to) {
            value = to_low * pow(10.0, normalized * log10(to_high / to_low));
        } else {
            value = normalized * (to_high - to_low) + to_low;
        }
        return value;
    }

    double distanceToLine(QPointF point, QPointF l1, QPointF l2, QPointF *closestLinePoint = nullptr, double *pointRatio = nullptr);

    // intensity color scale, input value from 0.0 to 1.0
    QColor getIntensityGradeColor(double intensity);
}

#endif // UTILH_H
