#ifndef LABEL_H
#define LABEL_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QPair>
#include <QPointF>
#include "savable.h"

// An annotation drawn in the PCB view. Either a plain text label anchored at a
// point, or a dimension (arrow spanning two points with a text label). All
// coordinates are stored as expression strings (number or equation over the
// parameters) and resolved to points against the parameter symbol table, mirroring
// the way Element handles its vertices.
class Label : public QObject, public Savable
{
    Q_OBJECT
public:
    enum class Type {
        Text,
        Dimension,
        Last,
    };

    explicit Label(Type type = Type::Text);

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    static QString TypeToString(Type type);
    static Type TypeFromString(QString s);
    static QList<Type> getTypes();
    // Number of anchor points a label of the given type needs (Text: 1, Dimension: 2).
    static int pointCountForType(Type type);

    Type getType() const {return type;}
    QString getText() const {return text;}
    int pointCount() const {return pointExpr.size();}
    // The (x, y) expression pair for an anchor point.
    QPair<QString, QString> getPointExpr(int index) const;
    // Resolved anchor coordinates (evaluated from the expressions).
    const QList<QPointF>& getPoints() const {return points;}

    void setText(QString s) {text = s;}
    // Changes the label type, resizing the anchor-point list to match.
    void setType(Type t);
    // Replaces the anchor-point expressions and evaluates them against the symbol table.
    void setPointExpressions(const QList<QPair<QString, QString>> &exprs, const QMap<QString, double> &symbols);
    // Recomputes the resolved points from their expressions. Invalid expressions
    // keep the previously resolved coordinate.
    void reevaluate(const QMap<QString, double> &symbols);

signals:
    void typeChanged();

private:
    void resizePoints(int count);

    Type type;
    QString text;
    QList<QPair<QString, QString>> pointExpr;   // (x, y) expressions
    QList<QPointF> points;                       // resolved coordinates, in lock-step
};

#endif // LABEL_H
