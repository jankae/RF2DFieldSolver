#include "labellist.h"

#include <QComboBox>

LabelList::LabelList(QObject *parent)
    : QAbstractTableModel{parent}
{

}

nlohmann::json LabelList::toJSON()
{
    nlohmann::json j;
    nlohmann::json jlabels;
    for(auto l : labels) {
        jlabels.push_back(l->toJSON());
    }
    j["labels"] = jlabels;
    return j;
}

void LabelList::fromJSON(nlohmann::json j)
{
    while(labels.size()) {
        removeLabel(0);
    }
    if(j.contains("labels")) {
        for(auto &jlabel : j["labels"]) {
            auto l = new Label(Label::Type::Text);
            l->fromJSON(jlabel);
            addLabel(l);
        }
    }
}

void LabelList::addLabel(Label *l)
{
    beginInsertRows(QModelIndex(), labels.size(), labels.size());
    labels.append(l);
    connect(l, &Label::typeChanged, this, [=](){
        auto i = findIndex(l);
        if(i != -1) {
            emit dataChanged(index(i, 0), index(i, (int) Column::Last - 1));
        }
    });
    connect(l, &Label::destroyed, this, [=](){
        removeLabel(l, false);
    });
    endInsertRows();
}

bool LabelList::removeLabel(Label *l, bool del)
{
    int i = findIndex(l);
    if(i != -1) {
        return removeLabel(i, del);
    } else {
        return false;
    }
}

bool LabelList::removeLabel(int index, bool del)
{
    if (index < 0 || index >= labels.size()) {
        return false;
    }
    beginRemoveRows(QModelIndex(), index, index);
    auto l = labels[index];
    labels.removeAt(index);
    disconnect(l, nullptr, this, nullptr);
    if(del) {
        delete l;
    }
    endRemoveRows();
    return true;
}

Label *LabelList::labelAt(int index) const
{
    if (index >= 0 && index < labels.size()) {
        return labels[index];
    } else {
        return nullptr;
    }
}

void LabelList::reevaluateAll(const QMap<QString, double> &symbols)
{
    for(auto l : labels) {
        l->reevaluate(symbols);
    }
}

QVariant LabelList::data(const QModelIndex &index, int role) const
{
    auto row = index.row();
    auto col = index.column();
    Label *l = labels[row];
    switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch((Column) col) {
        case Column::Type: return Label::TypeToString(l->getType());
        case Column::Text: return l->getText();
        case Column::Last: return QVariant();
        }
        break;
    default: return QVariant();
    }
    return QVariant();
}

QVariant LabelList::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical) {
        return QVariant();
    }
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    switch(section) {
    case 0: return "Type";
    case 1: return "Text";
    default: return QVariant();
    }
}

bool LabelList::setData(const QModelIndex &index, const QVariant &value, int role)
{
    auto row = index.row();
    auto col = index.column();
    Label *l = labels[row];
    switch(role) {
    case Qt::EditRole:
        switch((Column) col) {
        case Column::Type: l->setType(Label::TypeFromString(value.toString())); return true;
        case Column::Text: l->setText(value.toString()); emit dataChanged(index, index); return true;
        case Column::Last: return false;
        }
        break;
    }
    return false;
}

Qt::ItemFlags LabelList::flags(const QModelIndex &index) const
{
    auto flags = QAbstractTableModel::flags(index);
    flags |= Qt::ItemIsEditable;
    return flags;
}

int LabelList::findIndex(Label *l)
{
    return labels.indexOf(l);
}

QWidget *LabelTypeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    auto model = (LabelList*) index.model();
    auto editor = new QComboBox(parent);
    connect(editor, qOverload<int>(&QComboBox::currentIndexChanged), [editor](int) {
        editor->clearFocus();
    });
    for(auto t : Label::getTypes()) {
        editor->addItem(Label::TypeToString(t));
    }
    editor->setCurrentIndex((int) model->labelAt(index.row())->getType());
    return editor;
}

void LabelTypeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto l = static_cast<const LabelList*>(index.model())->labelAt(index.row());
    auto c = (QComboBox*) editor;
    l->setType(Label::TypeFromString(c->currentText()));
}

void LabelTypeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto list = (LabelList*) model;
    auto c = (QComboBox*) editor;
    list->setData(index, c->currentText());
}
