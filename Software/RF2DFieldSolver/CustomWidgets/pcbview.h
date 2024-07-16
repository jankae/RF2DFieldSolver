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

signals:

protected:
    void paintEvent(QPaintEvent *event) override;
private:
    static const QColor backgroundColor;
    static const QColor GNDColor;
    static const QColor traceColor;
    static const QColor dielectricColor;
    static constexpr int vertexSize = 10;
    QPointF topLeft;
    QPointF bottomRight;
    ElementList *list;
};

#endif // PCBVIEW_H
