#ifndef ELEMENT_H
#define ELEMENT_H

#include <QObject>
#include <QList>
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
    const QList<QPointF>& getVertices() const {return vertices;}
    void addVertex(int index, QPointF vertex);
    void appendVertex(QPointF vertex);
    void removeVertex(int index);
    void changeVertex(int index, QPointF newCoords);

    void setName(QString s) {name = s;}
    void setType(Type t);
    void setEpsilonR(double er) {epsilon_r = er;}
    QPolygonF toPolygon();

signals:
    void typeChanged();

private:
    QList<QPointF> vertices;
    QString name;
    Type type;
    double epsilon_r;
};

#endif // ELEMENT_H
