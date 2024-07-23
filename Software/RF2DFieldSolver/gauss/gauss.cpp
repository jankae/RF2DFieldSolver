#include "gauss.h"

#include "polygon.h"

Gauss::Gauss(QObject *parent)
    : QObject{parent}
{

}

double Gauss::getCharge(Laplace *laplace, ElementList *list, Element *e, double gridSize, double distance)
{
    // extend the element polygon a bit
    auto integral = Polygon::offset(e->getVertices(), distance);

    double chargeSum = 0;
    for(unsigned int i=0;i<integral.size();i++) {
        auto pp = integral[(i+integral.size()-1) % integral.size()];
        auto pc = integral[i];

        auto increment = QLineF(pp, pc);
        auto unitVector = increment;
        unitVector.setLength(1.0);
        unsigned int points = ceil(increment.length() / gridSize);
        double stepSize = increment.length() / points;
        increment.setLength(stepSize);
        auto point = pp + QPointF(increment.dx() / 2, increment.dy() / 2);
        for(unsigned int j=0;j<points;j++) {
            QLineF gradient = laplace->getGradient(point);
            if(list) {
                gradient.setLength(gradient.length() * list->getDielectricConstantAt(point));
            }
            // get amount of gradient that is perpendicular to our integration line
            double perp = gradient.dx() * unitVector.dy() - gradient.dy() * unitVector.dx();
            perp *= stepSize / gridSize;
            chargeSum += perp;
            point += QPointF(increment.dx(), increment.dy());
        }
    }

    if(!Polygon::isClockwise(integral)) {
        chargeSum *= -1;
    }

    return chargeSum;
}
