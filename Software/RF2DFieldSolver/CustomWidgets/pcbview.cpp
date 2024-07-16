#include "pcbview.h"

#include <QPainter>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>

#include "util.h"

const QColor PCBView::backgroundColor = Qt::lightGray;
const QColor PCBView::GNDColor = Qt::black;
const QColor PCBView::traceColor = Qt::red;
const QColor PCBView::dielectricColor = Qt::darkGreen;

PCBView::PCBView(QWidget *parent)
    : QWidget{parent}
{
    list = nullptr;
    topLeft = QPointF(-1, 1);
    topLeft = QPointF(1, -1);
    appendElement = nullptr;
    dragVertex.e = nullptr;
    dragVertex.index = 0;
    pressCoordsValid = false;
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

void PCBView::startAppending(Element *e)
{
    appendElement = e;
    setMouseTracking(true);
}

void PCBView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);

    p.setViewport(0, 0, width(), height());
    p.fillRect(0, 0, width(), height(), Qt::lightGray);
    // p.setWindow is limited to integers, this implementation works with floating point
    transform = QTransform();
    transform.scale(p.viewport().width() / (bottomRight.x() - topLeft.x()),
                    p.viewport().height() / (bottomRight.y() - topLeft.y()));
    transform.translate(-topLeft.x(), -topLeft.y());
    //p.setTransform(transform);

    //p.setWindow(topLeft.x(), topLeft.y(), bottomRight.x() - topLeft.x(), bottomRight.y() - topLeft.y());
  //  transform = p.combinedTransform();

    p.setBackground(QBrush(Qt::black));


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
                        if(e == appendElement) {
                            // we are appending to this element, do not draw last line in polygon
                            continue;
                        }
                        prev = vertices.size() -  1;
                    }
                    QPointF start = transform.map(vertices[i]);
                    QPointF stop = transform.map(vertices[prev]);
                    p.drawLine(start, stop);
                }
            }
            if(vertices.size() > 0 && e == appendElement){
                        // draw line from last vertex to pointer
                        QPointF start = transform.map(vertices[vertices.size()-1]);
                        QPointF stop = lastMouseCoords;
                        p.drawLine(start, stop);
            }
            p.setViewTransformEnabled(true);
        }
    }
}

void PCBView::mousePressEvent(QMouseEvent *event)
{
    if (appendElement) {
        // check if we clicked on the first vertex
        auto vertices = appendElement->getVertices();
        if(vertices.size() > 0 && getPixelDistanceToVertex(event->pos(), vertices[0]) < vertexCatchRadius) {
            // clicked on the first element again, abort append mode
            appendElement = nullptr;
            setMouseTracking(false);
            update();
        } else {
            // record coordinates to place vertex when the mouse is released
            pressCoords = event->pos();
            lastMouseCoords = pressCoords;
            pressCoordsValid = true;
        }
    } else {
        // not appending, may have been a click on a vertex
        dragVertex = catchVertex(event->pos());
    }
}

void PCBView::mouseReleaseEvent(QMouseEvent *event)
{
    if (appendElement && pressCoordsValid) {
        // add vertex at indicated coordinates
        auto vertexPoint = transform.inverted().map(QPointF(pressCoords));
        appendElement->appendVertex(vertexPoint);
        pressCoordsValid = false;
        update();
    } else if (dragVertex.e) {
        dragVertex.e = nullptr;
    }
}

void PCBView::mouseMoveEvent(QMouseEvent *event)
{
    if (appendElement) {
        // ignore, just record mouse position
        lastMouseCoords = event->pos();
        update();
    } else if(dragVertex.e) {
        // dragging a vertex
        auto vertexPoint = transform.inverted().map(QPointF(event->pos()));
        dragVertex.e->changeVertex(dragVertex.index, vertexPoint);
        update();
    }
}

void PCBView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (appendElement) {
        // double-click aborts appending
        appendElement = nullptr;
        setMouseTracking(false);
        update();
    }
}

void PCBView::contextMenuEvent(QContextMenuEvent *event)
{
    if (appendElement) {
        // ignore
        return;
    }
    auto menu = new QMenu();
    auto infoVertex = catchVertex(event->pos());
    auto infoLine = catchLine(event->pos());
    Element *e = nullptr;
    if(infoVertex.e) {
        e = infoVertex.e;
        // clicked on a vertex
        auto actionDeleteVertex = new QAction("Delete Vertex");
        menu->addAction(actionDeleteVertex);
        connect(actionDeleteVertex, &QAction::triggered, [=](){
            infoVertex.e->removeVertex(infoVertex.index);
        });
    } else if(infoLine.e) {
        e = infoLine.e;
        // clicked on a line
        auto actionInsertVertex = new QAction("Insert Vertex here");
        menu->addAction(actionInsertVertex);
        connect(actionInsertVertex, &QAction::triggered, [=](){
            // figure out at which index we have to insert the new vertex.
            // Usually this is the higher of the two but there is a special case if one index is the first vertex and the other the last
            int insertIndex = std::max(infoLine.index1, infoLine.index2);
            if((infoLine.index1 == 0 && infoLine.index2 == infoLine.e->getVertices().size() - 1)
                    || (infoLine.index1 == 0 && infoLine.index2 == infoLine.e->getVertices().size() - 1)) {
                // special case
                insertIndex++;
            }
            auto vertexPoint = transform.inverted().map(QPointF(event->pos()));
            infoLine.e->addVertex(insertIndex, vertexPoint);
        });
    }
    // TODO check if connection between vertices was clicked
    if(e) {
        // clicked on something connected to an element
        auto actionDeleteElement = new QAction("Delete Element");
        menu->addAction(actionDeleteElement);
        connect(actionDeleteElement, &QAction::triggered, [=](){
            list->removeElement(e);
        });
    }
    menu->exec(event->globalPos());
    update();
}

double PCBView::getPixelDistanceToVertex(QPoint cursor, QPointF vertex)
{
    // convert vertex into pixel coordinates
    QPoint vertexPixel = transform.map(vertex).toPoint();
    auto diff = vertexPixel - cursor;
    return std::sqrt(diff.x()*diff.x()+diff.y()*diff.y());
}

PCBView::VertexInfo PCBView::catchVertex(QPoint cursor)
{
    VertexInfo info;
    info.e = nullptr;
    double closestDistance = vertexCatchRadius;
    for(auto e : list->getElements()) {
        for(unsigned int i=0;i<e->getVertices().size();i++) {
            auto distance = getPixelDistanceToVertex(cursor, e->getVertices()[i]);
            if(distance < closestDistance) {
                closestDistance = distance;
                info.e = e;
                info.index = i;
            }
        }
    }
    return info;
}

PCBView::LineInfo PCBView::catchLine(QPoint cursor)
{
    LineInfo info;
    info.e = nullptr;
    double closestDistance = vertexCatchRadius;
    for(auto e : list->getElements()) {
        if(e->getVertices().size() < 2) {
            continue;
        }
        for(unsigned int i=0;i<e->getVertices().size();i++) {
            int prev = (int) i - 1;
            if(prev < 0) {
                prev = e->getVertices().size() - 1;
            }
            QPointF vertexPixel1 = transform.map(e->getVertices()[i]).toPoint();
            QPointF vertexPixel2 = transform.map(e->getVertices()[prev]).toPoint();
            auto distance = Util::distanceToLine(QPointF(cursor), vertexPixel1, vertexPixel2);
            if(distance < closestDistance) {
                closestDistance = distance;
                info.e = e;
                info.index1 = i;
                info.index2 = prev;
            }
        }
    }
    return info;
}
