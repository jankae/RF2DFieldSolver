#include "label.h"

#include "expression.h"

#include <cmath>

// Full-precision literal (in base SI units, i.e. meters) for a resolved coordinate.
static QString literalExpr(double v)
{
    return QString::number(v, 'g', 12);
}

Label::Label(Type type)
    : QObject{nullptr},
      type(type)
{
    switch(type) {
    case Type::Text: text = "text"; break;
    case Type::Dimension: text = "dim"; break;
    case Type::Last: break;
    }
    resizePoints(pointCountForType(type));
}

int Label::pointCountForType(Type type)
{
    switch(type) {
    case Type::Text: return 1;
    case Type::Dimension: return 2;
    case Type::Last: return 0;
    }
    return 0;
}

void Label::resizePoints(int count)
{
    while(pointExpr.size() > count) {
        pointExpr.removeLast();
        points.removeLast();
    }
    while(pointExpr.size() < count) {
        pointExpr.append(QPair<QString, QString>(QStringLiteral("0"), QStringLiteral("0")));
        points.append(QPointF(0, 0));
    }
}

nlohmann::json Label::toJSON()
{
    nlohmann::json j;
    j["type"] = TypeToString(type).toStdString();
    j["text"] = text.toStdString();
    nlohmann::json jpoints;
    for(auto &e : pointExpr) {
        nlohmann::json jpoint;
        // store the raw expression strings so units survive the round-trip
        jpoint["x"] = e.first.toStdString();
        jpoint["y"] = e.second.toStdString();
        jpoints.push_back(jpoint);
    }
    j["points"] = jpoints;
    return j;
}

void Label::fromJSON(nlohmann::json j)
{
    type = TypeFromString(QString::fromStdString(j.value("type", "Text")));
    text = QString::fromStdString(j.value("text", text.toStdString()));
    pointExpr.clear();
    points.clear();
    // Accept both the string-expression form and a plain numeric form.
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
    if(j.contains("points")) {
        for(auto jpoint : j["points"]) {
            pointExpr.push_back({readCoord(jpoint, "x"), readCoord(jpoint, "y")});
            points.push_back(QPointF(0, 0));
        }
    }
    // make sure the point count is consistent with the type
    resizePoints(pointCountForType(type));
    // resolve any pure-numeric coordinates immediately; expressions referencing
    // parameters are resolved later once the parameter symbol table is available
    reevaluate(QMap<QString, double>());
}

QString Label::TypeToString(Type type)
{
    switch(type) {
    case Type::Text: return "Text";
    case Type::Dimension: return "Dimension";
    case Type::Last: return "";
    }
    return "";
}

Label::Type Label::TypeFromString(QString s)
{
    for(unsigned int i=0;i<(int) Type::Last;i++) {
        if(s == TypeToString((Type) i)) {
            return (Type) i;
        }
    }
    return Type::Text;
}

QList<Label::Type> Label::getTypes()
{
    QList<Type> ret;
    for(unsigned int i=0;i<(int) Type::Last;i++) {
        ret.append((Type) i);
    }
    return ret;
}

QPair<QString, QString> Label::getPointExpr(int index) const
{
    if(index >= 0 && index < pointExpr.size()) {
        return pointExpr[index];
    }
    return {QString("0"), QString("0")};
}

void Label::setType(Type t)
{
    if(type == t) {
        return;
    }
    type = t;
    resizePoints(pointCountForType(type));
    emit typeChanged();
}

void Label::setPointExpressions(const QList<QPair<QString, QString>> &exprs, const QMap<QString, double> &symbols)
{
    pointExpr = exprs;
    points.clear();
    for(int i=0;i<pointExpr.size();i++) {
        points.append(QPointF(0, 0));
    }
    reevaluate(symbols);
}

void Label::reevaluate(const QMap<QString, double> &symbols)
{
    for(int i=0;i<pointExpr.size();i++) {
        double x = Expression::evaluate(pointExpr[i].first, symbols, nullptr);
        double y = Expression::evaluate(pointExpr[i].second, symbols, nullptr);
        // keep the previous resolved value for invalid expressions
        if(!std::isnan(x)) {
            points[i].setX(x);
        }
        if(!std::isnan(y)) {
            points[i].setY(y);
        }
    }
}
