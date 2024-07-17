#include "elementlist.h"

#include <QComboBox>

ElementList::ElementList(QObject *parent)
    : QAbstractTableModel{parent}
{

}

void ElementList::addElement(Element *e)
{
    beginInsertRows(QModelIndex(), elements.size(), elements.size());
    elements.append(e);
    // TODO set up connections
    connect(e, &Element::typeChanged, this, [=](){
        auto i = findIndex(e);
        if(i != -1) {
            emit dataChanged(index(i, (int) Column::EpsilonR), index(i, (int) Column::EpsilonR));
        }
    });
    connect(e, &Element::destroyed, this, [=](){
        removeElement(e, false);
    });
    endInsertRows();
}

bool ElementList::removeElement(Element *e, bool del)
{
    int i = findIndex(e);
    if(i != -1) {
        return removeElement(i, del);
    } else {
        // not found
        return false;
    }
}

bool ElementList::removeElement(int index, bool del)
{
    if (index < 0 || index >= elements.size()) {
        return false;
    }
    beginRemoveRows(QModelIndex(), index, index);
    auto e = elements[index];
    elements.removeAt(index);
    disconnect(e, nullptr, this, nullptr);
    if(del) {
        delete e;
    }
    endRemoveRows();
    return true;
}

Element *ElementList::elementAt(int index) const
{
    if (index >= 0 || index < elements.size()) {
        return elements[index];
    } else {
        return nullptr;
    }
}

double ElementList::getDielectricConstantAt(const QPointF &p)
{
    for(unsigned int i=0;i<elements.size();i++) {
        auto e = elements[i];
        QPolygonF poly = QPolygonF(e->getVertices());
        if(poly.containsPoint(p, Qt::OddEvenFill)) {
            // this polygon defines the weight at these coordinates
            switch(e->getType()) {
            case Element::Type::GND:
            case Element::Type::Trace:
                return 1.0;
            case Element::Type::Dielectric:
                return e->getEpsilonR();
            case Element::Type::Last:
                return 1.0;
            }
        }
    }
    // not found, we are in the air
    return 1.0;
}

QVariant ElementList::data(const QModelIndex &index, int role) const
{
    auto row = index.row();
    auto col = index.column();
    Element *e = elements[row];
    switch(role) {
    case Qt::DisplayRole:
        switch((Column) col) {
        case Column::Name: return e->getName();
        case Column::Type: return e->TypeToString(e->getType());
        case Column::EpsilonR:
            if(e->getType() == Element::Type::Dielectric) {
                return QString::number(e->getEpsilonR());
            } else {
                return "";
            }
        case Column::Last: return QVariant();
        }
        break;
    default: return QVariant();
    }
    return QVariant();
}

QVariant ElementList::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical) {
        // only horizontal header
        return QVariant();
    }
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    switch(section) {
    case 0: return "Name";
    case 1: return "Type";
    case 2: return "εr";
    default: return QVariant();
    }
}

bool ElementList::setData(const QModelIndex &index, const QVariant &value, int role)
{
    auto row = index.row();
    auto col = index.column();
    Element *e = elements[row];
    switch(role) {
    case Qt::EditRole:
        switch((Column) col) {
        case Column::Name: e->setName(value.toString()); return true;
        case Column::Type: e->setType(Element::TypeFromString(value.toString())); return true;
        case Column::EpsilonR:
            if(e->getType() == Element::Type::Dielectric) {
                e->setEpsilonR(value.toDouble());
                return true;
            } else {
                return false;
            }
        case Column::Last: return false;
        }
        break;
    }

    return false;
}

Qt::ItemFlags ElementList::flags(const QModelIndex &index) const
{
    auto flags = QAbstractTableModel::flags(index);

    bool editable = false;
    auto row = index.row();
    auto col = index.column();
    Element *e = elements[row];
    switch((Column) col) {
    case Column::Name: editable = true; break;
    case Column::Type: editable = true; break;
    case Column::EpsilonR: editable = e->getType() == Element::Type::Dielectric; break;
    case Column::Last: break;
    }
    if (editable) {
        flags |= Qt::ItemIsEditable;
    } else {
        flags &= ~Qt::ItemIsSelectable;
    }
    return flags;
}

int ElementList::findIndex(Element *e)
{
    return elements.indexOf(e);
}

QWidget *TypeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    auto model = (ElementList*) index.model();
    auto editor = new QComboBox(parent);
    //c->setMaximumHeight(rowHeight);
    connect(editor, qOverload<int>(&QComboBox::currentIndexChanged), [editor](int) {
        editor->clearFocus();
    });
    for(auto t : Element::getTypes()) {
        editor->addItem(Element::TypeToString(t));
    }
    editor->setCurrentIndex((int) model->elementAt(index.row())->getType());
    return editor;
}

void TypeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto e = static_cast<const ElementList*>(index.model())->elementAt(index.row());
    auto c = (QComboBox*) editor;
    e->setType(Element::TypeFromString(c->currentText()));
}

void TypeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto list = (ElementList*) model;
    auto c = (QComboBox*) editor;
    list->setData(index, c->currentText());
}
