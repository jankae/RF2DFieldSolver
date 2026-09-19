#ifndef PCBVIEW_H
#define PCBVIEW_H

#include <QWidget>

#include <QPointF>
#include "elementlist.h"
#include "labellist.h"
#include "parameterlist.h"
#include "laplace/laplace.h"

class PCBView : public QWidget
{
    Q_OBJECT
public:
    explicit PCBView(QWidget *parent = nullptr);

    void setCorners(QPointF topLeft, QPointF bottomRight);
    void setElementList(ElementList *list);
    void setLabelList(LabelList *labels);
    void setParameters(ParameterList *params);
    void setLaplace(Laplace *laplace);
    // Highlights the given element in the view (nullptr clears the highlight).
    void setSelectedElement(Element *e);

    void startAppending(Element *e);
    // Ends any in-progress click-to-draw session (e.g. when the user switches
    // to defining points manually via the points dialog).
    void stopAppending();
    void setGrid(double grid);
    void setShowGrid(bool show);
    void setSnapToGrid(bool snap);
    void setShowPotential(bool show);
    void setKeepAspectRatio(bool keep);
    void setShowLabels(bool show);
    void setFillContours(bool fill);
    void setLabelTextSize(int pixels);

    bool getShowLabels() const {return showLabels;}
    bool getFillContours() const {return fillContours;}
    int getLabelTextSize() const {return labelTextSize;}

    QPointF getTopLeft() const;

    QPointF getBottomRight() const;

signals:
    // Emitted when the user clicks an element in the view (nullptr on empty space).
    void elementSelected(Element *e);

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
    static const QColor tracePosColor;
    static const QColor traceNegColor;
    static const QColor dielectricColor;
    static const QColor gridColor;
    static constexpr int vertexSize = 10;
    static constexpr int vertexCatchRadius = 15;
    double getPixelDistanceToVertex(QPoint cursor, QPointF vertex);
    void someElementChanged();

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

    // Returns the element under the cursor (vertex, edge or interior), or nullptr.
    Element *elementAtCursor(QPoint cursor);

    QPointF topLeft;
    QPointF bottomRight;
    QTransform transform;
    ElementList *list;
    LabelList *labelList;
    ParameterList *params;
    Laplace *laplace;

    Element *selectedElement;
    Element *appendElement;
    VertexInfo dragVertex;

    QPoint pressCoords;
    QPoint lastMouseCoords;
    bool pressCoordsValid;

    QPointF snapToGridPoint(const QPointF &pos);
    double grid;
    bool showGrid;
    bool snapToGrid;
    bool showPotential;
    bool keepAspectRatio;
    bool showLabels;
    bool fillContours;
    int labelTextSize;
};

#endif // PCBVIEW_H
