#include "pcbview.h"

#include <QPainter>

const QColor PCBView::backgroundColor = Qt::lightGray;
const QColor PCBView::GNDColor = Qt::black;
const QColor PCBView::traceColor = Qt::red;
const QColor PCBView::dielectricColor = Qt::darkGreen;

PCBView::PCBView(QWidget *parent)
    : QWidget{parent}
{
    list = nullptr;
}

void PCBView::setCorners(QPointF topLeft, QPointF bottomRight)
{
    this->topLeft = topLeft;
    this->bottomRight = bottomRight;
}

void PCBView::setElementList(ElementList *list)
{
    this->list = list;
}

void PCBView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);

    p.setViewport(0, 0, width(), height());
    p.setWindow(topLeft.x(), topLeft.y(), bottomRight.x() - topLeft.x(), bottomRight.y() - topLeft.y());


    p.setBackground(QBrush(Qt::black));
    p.fillRect(QRectF(topLeft, bottomRight), Qt::lightGray);

    // Show elements
    if(list) {
        for(auto e : list->getElements()) {
            QColor elementColor;
            switch(e->getType()) {
            case Element::Type::Dielectric: elementColor = dielectricColor; break;
            case Element::Type::Trace: elementColor = traceColor; break;
            case Element::Type::GND: elementColor = GNDColor; break;
            default: elementColor = Qt::gray; break;
            }
            p.setBrush(elementColor);
            p.setPen(elementColor);

            auto vertices = e->getVertices();
            // paint vertices in viewport to get constant vertex size
            auto transform = p.combinedTransform();
            auto w = width();
            auto h = height();
            p.setViewTransformEnabled(false);
            for(auto v : vertices) {
                auto devicePoint = transform.map(v);
                p.drawEllipse(devicePoint, vertexSize/2, vertexSize/2);
            }
            // draw connections between vertices
            if(vertices.size() > 1) {
                for(unsigned int i=0;i<vertices.size();i++) {
                    int prev = i-1;
                    if(prev < 0) {
                        prev = vertices.size() -  1;
                    }
                    QPointF start = transform.map(vertices[i]);
                    QPointF stop = transform.map(vertices[prev]);
                    p.drawLine(start, stop);
                }
            }
            p.setViewTransformEnabled(true);
            qDebug() << h << w;
        }
    }
}
