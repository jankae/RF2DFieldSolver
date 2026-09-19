#include "parameterlist.h"

#include "expression.h"
#include "unit.h"
#include "CustomWidgets/informationbox.h"

#include <QBrush>
#include <QColor>
#include <cmath>
#include <limits>

// Ascending SI prefixes with the base unit (space) in its correct 1e0 position,
// as required by Unit::ToString. Used to preview evaluated parameter values.
static const QString VALUE_PREFIXES = "fpnum kMGTP";
// Geometry parameters are lengths; show the metre unit so the value is
// unambiguous (e.g. "18.5mm" rather than a bare "18.5m" milli prefix), matching
// every other length field in the application.
static const QString VALUE_UNIT = "m";

ParameterList::ParameterList(QObject *parent)
    : QAbstractTableModel{parent}
{

}

nlohmann::json ParameterList::toJSON()
{
    nlohmann::json j;
    nlohmann::json jparams;
    for(auto &p : params) {
        nlohmann::json jp;
        jp["name"] = p.name.toStdString();
        // store the raw expression text so units survive the round-trip
        jp["expression"] = p.expression.toStdString();
        jparams.push_back(jp);
    }
    j["parameters"] = jparams;
    return j;
}

void ParameterList::fromJSON(nlohmann::json j)
{
    beginResetModel();
    params.clear();
    if(j.contains("parameters")) {
        for(auto &jp : j["parameters"]) {
            Parameter p;
            p.name = QString::fromStdString(jp.value("name", std::string()));
            p.expression = QString::fromStdString(jp.value("expression", std::string()));
            p.value = std::numeric_limits<double>::quiet_NaN();
            p.valid = false;
            params.append(p);
        }
    }
    reevaluate();
    endResetModel();
    emit parametersChanged();
}

void ParameterList::addParameter(const QString &name, const QString &expression)
{
    beginInsertRows(QModelIndex(), params.size(), params.size());
    Parameter p;
    p.name = name.isEmpty() ? uniqueName() : name;
    p.expression = expression;
    p.value = std::numeric_limits<double>::quiet_NaN();
    p.valid = false;
    params.append(p);
    endInsertRows();
    reevaluate();
    if(params.size() > 0) {
        emit dataChanged(index(0, (int) Column::Value), index(params.size()-1, (int) Column::Value));
    }
    emit parametersChanged();
}

void ParameterList::removeParameter(int idx)
{
    if(idx < 0 || idx >= params.size()) {
        return;
    }
    beginRemoveRows(QModelIndex(), idx, idx);
    params.removeAt(idx);
    endRemoveRows();
    reevaluate();
    if(params.size() > 0) {
        emit dataChanged(index(0, (int) Column::Value), index(params.size()-1, (int) Column::Value));
    }
    emit parametersChanged();
}

void ParameterList::reevaluate()
{
    symbolCache.clear();
    for(auto &p : params) {
        if(p.expression.trimmed().isEmpty()) {
            // a not-yet-filled-in parameter: neither valid nor an error
            p.valid = false;
            p.value = std::numeric_limits<double>::quiet_NaN();
            p.error.clear();
            continue;
        }
        QString err;
        double v = Expression::evaluate(p.expression, symbolCache, &err);
        if(std::isnan(v)) {
            p.valid = false;
            p.value = v;
            p.error = err;
        } else {
            p.valid = true;
            p.value = v;
            p.error.clear();
            // only valid, named parameters become referenceable by later rows
            if(!p.name.isEmpty()) {
                symbolCache.insert(p.name, v);
            }
        }
    }
}

QString ParameterList::uniqueName() const
{
    int n = 1;
    while(true) {
        QString candidate = QString("param%1").arg(n);
        bool taken = false;
        for(auto &p : params) {
            if(p.name == candidate) {
                taken = true;
                break;
            }
        }
        if(!taken) {
            return candidate;
        }
        n++;
    }
}

QVariant ParameterList::data(const QModelIndex &index, int role) const
{
    if(!index.isValid() || index.row() >= params.size()) {
        return QVariant();
    }
    const Parameter &p = params[index.row()];
    switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch((Column) index.column()) {
        case Column::Name: return p.name;
        case Column::Expression: return p.expression;
        case Column::Value:
            if(role == Qt::EditRole) {
                return QVariant();
            }
            if(p.valid) {
                return Unit::ToString(p.value, VALUE_UNIT, VALUE_PREFIXES, 4);
            } else if(!p.error.isEmpty()) {
                return QString("error");
            } else {
                return QVariant();
            }
        case Column::Last: return QVariant();
        }
        break;
    case Qt::ToolTipRole:
        if(!p.valid && !p.error.isEmpty()) {
            return p.error;
        }
        break;
    case Qt::ForegroundRole:
        if(!p.valid && !p.error.isEmpty()) {
            return QBrush(QColor(200, 0, 0));
        }
        break;
    }
    return QVariant();
}

QVariant ParameterList::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Vertical) {
        return QVariant();
    }
    if(role != Qt::DisplayRole) {
        return QVariant();
    }
    switch((Column) section) {
    case Column::Name: return "Name";
    case Column::Expression: return "Expression";
    case Column::Value: return "Value";
    default: return QVariant();
    }
}

bool ParameterList::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role != Qt::EditRole || !index.isValid() || index.row() >= params.size()) {
        return false;
    }
    Parameter &p = params[index.row()];
    switch((Column) index.column()) {
    case Column::Name: {
        QString newName = value.toString();
        if(newName == p.name) {
            return false;
        }
        // the name must be usable as an identifier in an expression and must not
        // collide with a reserved function name
        if(!Expression::isValidParameterName(newName)) {
            InformationBox::ShowError("Invalid parameter name",
                "\"" + newName + "\" cannot be used as a parameter name. Use a letter or "
                "underscore followed by letters, digits or underscores, and avoid the "
                "reserved function names (" + Expression::functionNames().join(", ") + ").");
            return false;
        }
        // names must be unique so expressions resolve unambiguously
        for(int i=0;i<params.size();i++) {
            if(i != index.row() && params[i].name == newName) {
                InformationBox::ShowError("Duplicate parameter name",
                    "A parameter named \"" + newName + "\" already exists.");
                return false;
            }
        }
        p.name = newName;
        break;
    }
    case Column::Expression: p.expression = value.toString(); break;
    default: return false;
    }
    // a name or expression change can affect the value of any later parameter
    reevaluate();
    emit dataChanged(this->index(0, 0), this->index(params.size()-1, (int) Column::Last - 1));
    emit parametersChanged();
    return true;
}

Qt::ItemFlags ParameterList::flags(const QModelIndex &index) const
{
    auto f = QAbstractTableModel::flags(index);
    switch((Column) index.column()) {
    case Column::Name:
    case Column::Expression:
        f |= Qt::ItemIsEditable;
        break;
    default:
        break;
    }
    return f;
}
