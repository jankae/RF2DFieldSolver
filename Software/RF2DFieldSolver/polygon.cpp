#include "polygon.h"

#include <QLineF>

bool Polygon::selfIntersects(const QList<QPointF> &vertices)
{
    for(int i=0;i<(int) vertices.size();i++) {
        int prev = i-1;
        if(prev < 0) {
            prev = vertices.size() - 1;
        }
        auto p0 = vertices[prev];
        auto p1 = vertices[i];
        auto line0 = QLineF(p0, p1);
        for(int j=i+1;j<(int) vertices.size();j++) {
            int prev = j-1;
            if(prev < 0) {
                prev = vertices.size() - 1;
            }
            auto p2 = vertices[prev];
            auto p3 = vertices[j];
            auto line1 = QLineF(p2, p3);
            QPointF intersectPoint;
            auto intersect = line0.intersects(line1, &intersectPoint);
            if(intersect == QLineF::IntersectionType::BoundedIntersection) {
                // we have an intersection. Check whether we are checking consecutive lines and the intersect point is the common vertex
                if(j == i+1 || (i==0 && j==vertices.size() - 1)) {
                    QPointF common, pl1, pl2;
                    if(j == i+1) {
                        common = p1;
                        pl1 = p0;
                        pl2 = p3;
                    } else {
                        common = p3;
                        pl1 = p1;
                        pl2 = p2;
                    }
                    auto commonIntersectDist = QLineF(common, intersectPoint).length();
                    auto pl1IntersectDist = QLineF(pl1, intersectPoint).length();
                    auto pl2IntersectDist = QLineF(pl2, intersectPoint).length();
                    if(commonIntersectDist < pl1IntersectDist && commonIntersectDist < pl2IntersectDist) {
                        // ignore, this is not a real intersection
                        continue;
                    }
                }
                // real intersection
                return true;
            }
        }
    }
    // no intersection found
    return false;
}


QList<QPointF> Polygon::offset(const QList<QPointF> &vertices, double offset)
{
    QList<QPointF> ret;
    if (vertices.size() < 3) {
        // unable to offset
        return vertices;
    }

    if (isClockwise(vertices)) {
        // CW polygon, invert offset
        offset = -offset;
    }
    // offset lines and create new points at the intersect points:
    // https://stackoverflow.com/questions/69600158/draw-a-second-identical-polygon-inside-a-primary-one-with-a-certain-gap
    for(unsigned int i = 0;i<vertices.size();i++) {
        auto pp = vertices[(i+vertices.size()-1) % vertices.size()];
        auto pc = vertices[i];
        auto pn = vertices[(i+vertices.size()+1) % vertices.size()];

        auto line0 = QLineF(pp, pc);
        auto normal0 = line0.normalVector();
        normal0.setLength(offset);
        line0.translate(normal0.dx(), normal0.dy());

        auto line1 = QLineF(pc, pn);
        auto normal1 = line1.normalVector();
        normal1.setLength(offset);
        line1.translate(normal1.dx(), normal1.dy());

        QPointF point;
        auto intersect = line0.intersects(line1, &point);
        if (intersect != QLineF::IntersectType::NoIntersection) {
            ret.append(point);
        }
    }
    return ret;
}

bool Polygon::isClockwise(const QList<QPointF> &vertices)
{
    // determine winding direction of polygon:
    // https://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
    double edgeSum = 0;
    for(unsigned int i = 0;i<vertices.size();i++) {
        auto pp = vertices[(i+vertices.size()-1) % vertices.size()];
        auto pc = vertices[i];
        edgeSum += (pc.x() - pp.x())*(pc.y() + pp.y());
    }
    return edgeSum > 0;
}
