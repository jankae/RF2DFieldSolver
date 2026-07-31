#include "element.h"

#include "expression.h"

#include <cmath>

// Full-precision literal (in base SI units, i.e. meters) for a resolved
// coordinate, so that dragging/clicking a vertex round-trips exactly.
static QString literalExpr(double v)
{
    return QString::number(v, 'g', 12);
}

Element::Element(Type type)
    : QObject{nullptr},
      type(type)
{
    epsilon_r = 4.3;
    switch(type) {
    case Type::TracePos: name = "RF+"; break;
    case Type::TraceNeg: name = "RF-"; break;
    case Type::Dielectric: name = "Substrate"; break;
    case Type::GND: name = "GND"; break;
    case Type::Last: break;
    }
}

nlohmann::json Element::toJSON()
{
    nlohmann::json j;
    j["name"] = name.toStdString();
    j["type"] = TypeToString(type).toStdString();
    j["e_r"] = epsilon_r;
    nlohmann::json jvertices;
    for(auto &e : vertexExpr) {
        nlohmann::json jvertex;
        // store the raw expression strings so units survive the round-trip
        jvertex["x"] = e.first.toStdString();
        jvertex["y"] = e.second.toStdString();
        jvertices.push_back(jvertex);
    }
    j["vertices"] = jvertices;
    return j;
}

void Element::fromJSON(nlohmann::json j)
{
    name = QString::fromStdString(j.value("name", name.toStdString()));
    type = TypeFromString(QString::fromStdString(j.value("type", "")));
    epsilon_r = j.value("e_r", epsilon_r);
    vertices.clear();
    vertexExpr.clear();
    // Accept both the new string-expression form and the legacy numeric form so
    // that older project files continue to open.
    auto readCoord = [](const nlohmann::json &jv, const char *key) -> QString {
        if(!jv.contains(key)) {
            return "0";
        }
        const auto &e = jv[key];
        if(e.is_string()) {
            return QString::fromStdString(e.get<std::string>());
        }
        if(e.is_number()) {
            return literalExpr(e.get<double>());
        }
        return "0";
    };
    if(j.contains("vertices")) {
        for(auto jvertex : j["vertices"]) {
            vertexExpr.push_back({readCoord(jvertex, "x"), readCoord(jvertex, "y")});
            vertices.push_back(QPointF(0, 0));
        }
    }
    // resolve any pure-numeric vertices immediately; expressions referencing
    // parameters are resolved later once the parameter symbol table is available
    reevaluate(QMap<QString, double>());
}

QString Element::TypeToString(Type type)
{
    switch(type) {
    case Type::Dielectric: return "Dielectric";
    case Type::GND: return "GND";
    case Type::TracePos: return "Trace+";
    case Type::TraceNeg: return "Trace-";
    case Type::Last: return "";
    }
    return "";
}

Element::Type Element::TypeFromString(QString s)
{
    for(unsigned int i=0;i<(int) Type::Last;i++) {
        if(s == TypeToString((Type) i)) {
            return (Type) i;
        }
    }
    return Type::Last;
}

QList<Element::Type> Element::getTypes()
{
    QList<Type> ret;
    for(unsigned int i=0;i<(int) Type::Last;i++) {
        ret.append((Type) i);
    }
    return ret;
}

QPair<QString, QString> Element::getVertexExpr(int index) const
{
    if(index >= 0 && index < vertexExpr.size()) {
        return vertexExpr[index];
    }
    return {QString(), QString()};
}

void Element::addVertex(int index, QPointF vertex)
{
    vertices.insert(index, vertex);
    vertexExpr.insert(index, {literalExpr(vertex.x()), literalExpr(vertex.y())});
}

void Element::appendVertex(QPointF vertex)
{
    vertices.append(vertex);
    vertexExpr.append({literalExpr(vertex.x()), literalExpr(vertex.y())});
}

void Element::removeVertex(int index)
{
    if(index >= 0 && index < vertices.size()) {
        vertices.removeAt(index);
        vertexExpr.removeAt(index);
    }
}

void Element::changeVertex(int index, QPointF newCoords)
{
    if(index >= 0 && index < vertices.size()) {
        vertices[index] = newCoords;
        vertexExpr[index] = {literalExpr(newCoords.x()), literalExpr(newCoords.y())};
    }
}

void Element::setVertexExpr(int index, const QString &xExpr, const QString &yExpr)
{
    if(index >= 0 && index < vertexExpr.size()) {
        vertexExpr[index] = {xExpr, yExpr};
    }
}

void Element::setVertexExpressions(const QList<QPair<QString, QString>> &exprs, const QMap<QString, double> &symbols)
{
    vertexExpr = exprs;
    vertices.clear();
    for(int i=0;i<vertexExpr.size();i++) {
        vertices.append(QPointF(0, 0));
    }
    reevaluate(symbols);
}

void Element::reevaluate(const QMap<QString, double> &symbols)
{
    for(int i=0;i<vertexExpr.size();i++) {
        double x = Expression::evaluate(vertexExpr[i].first, symbols, nullptr);
        double y = Expression::evaluate(vertexExpr[i].second, symbols, nullptr);
        // keep the previous resolved value for invalid expressions
        if(!std::isnan(x)) {
            vertices[i].setX(x);
        }
        if(!std::isnan(y)) {
            vertices[i].setY(y);
        }
    }
}

void Element::setType(Type t)
{
    type = t;
    emit typeChanged();
}

QPolygonF Element::toPolygon()
{
    auto ret = QPolygonF(vertices);
//    if(vertices.size() > 2) {
//        // QPolygon expects the last point to be the same as the first
//        ret << vertices[0];
//    }
    return ret;
}

