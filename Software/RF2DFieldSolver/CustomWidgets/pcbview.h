#ifndef PCBVIEW_H
#define PCBVIEW_H

#include <QWidget>

#include <QPointF>
#include "elementlist.h"

class PCBView : public QWidget
{
    Q_OBJECT
public:
    explicit PCBView(QWidget *parent = nullptr);

    void setCorners(QPointF topLeft, QPointF bottomRight);
    void setElementList(ElementList *list);

    void startAppending(Element *e);

signals:

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
private:
    static const QColor backgroundColor;
    static const QColor GNDColor;
    static const QColor traceColor;
    static const QColor dielectricColor;
    static constexpr int vertexSize = 10;
    static constexpr int vertexCatchRadius = 15;
    double getPixelDistanceToVertex(QPoint cursor, QPointF vertex);

    using VertexInfo = struct {
        Element *e;
        int index;
    };
    VertexInfo catchVertex(QPoint cursor);

    using LineInfo = struct {
        Element *e;
        int index1, index2;
    };
    LineInfo catchLine(QPoint cursor);

    QPointF topLeft;
    QPointF bottomRight;
    QTransform transform;
    ElementList *list;

    Element *appendElement;
    VertexInfo dragVertex;

    QPoint pressCoords;
    QPoint lastMouseCoords;
    bool pressCoordsValid;
};

#endif // PCBVIEW_H
