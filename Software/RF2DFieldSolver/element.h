#ifndef ELEMENT_H
#define ELEMENT_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QPair>
#include <QPointF>
#include <QPolygonF>
#include "savable.h"

class Element : public QObject, public Savable
{
    Q_OBJECT
public:
    enum class Type {
        Dielectric,
        TracePos,
        TraceNeg,
        GND,
        Last,
    };

    explicit Element(Type type);

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    static QString TypeToString(Type type);
    static Type TypeFromString(QString s);
    static QList<Type> getTypes();

    QString getName() const {return name;}
    Type getType() const {return type;}
    double getEpsilonR() const {return epsilon_r;}
    // Resolved vertex coordinates (evaluated from the vertex expressions). This
    // is the source of truth for all geometry/simulation consumers.
    const QList<QPointF>& getVertices() const {return vertices;}
    int vertexCount() const {return vertexExpr.size();}
    // The (x, y) expression pair for a vertex.
    QPair<QString, QString> getVertexExpr(int index) const;

    // Raw-point mutators (used when clicking/dragging in the view): they store
    // the coordinate as a literal expression.
    void addVertex(int index, QPointF vertex);
    void appendVertex(QPointF vertex);
    void removeVertex(int index);
    void changeVertex(int index, QPointF newCoords);

    // Sets the (x, y) expression pair of a single vertex. The resolved
    // coordinate is refreshed on the next reevaluate().
    void setVertexExpr(int index, const QString &xExpr, const QString &yExpr);
    // Replaces the whole vertex list with expression pairs and evaluates them
    // against the given symbol table (used by the points edit dialog).
    void setVertexExpressions(const QList<QPair<QString, QString>> &exprs, const QMap<QString, double> &symbols);
    // Recomputes the resolved vertices from their expressions. Invalid
    // expressions keep the previously resolved coordinate.
    void reevaluate(const QMap<QString, double> &symbols);

    void setName(QString s) {name = s;}
    void setType(Type t);
    void setEpsilonR(double er) {epsilon_r = er;}
    QPolygonF toPolygon();

signals:
    void typeChanged();

private:
    QList<QPointF> vertices;                        // resolved coordinates
    QList<QPair<QString, QString>> vertexExpr;       // (x, y) expressions, kept in lock-step with vertices
    QString name;
    Type type;
    double epsilon_r;
};

#endif // ELEMENT_H
