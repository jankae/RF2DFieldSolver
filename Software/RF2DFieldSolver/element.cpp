#include "element.h"

Element::Element(Type type)
    : QObject{nullptr},
      type(type)
{
    epsilon_r = 4.3;
    switch(type) {
    case Type::Trace: name = "RF"; break;
    case Type::Dielectric: name = "Substrate"; break;
    case Type::GND: name = "GND"; break;
    case Type::Last: break;
    }
}

QString Element::TypeToString(Type type)
{
    switch(type) {
    case Type::Dielectric: return "Dielectric";
    case Type::GND: return "GND";
    case Type::Trace: return "Trace";
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

void Element::addVertex(int index, QPointF vertex)
{
    vertices.insert(index, vertex);
}

void Element::appendVertex(QPointF vertex)
{
    vertices.append(vertex);
}

void Element::changeVertex(int index, QPointF newCoords)
{
    if(index >= 0 && index < vertices.size()) {
        vertices[index] = newCoords;
    }
}

void Element::setType(Type t)
{
    type = t;
    emit typeChanged();
}

