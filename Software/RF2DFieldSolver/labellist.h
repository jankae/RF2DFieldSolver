#ifndef LABELLIST_H
#define LABELLIST_H

#include <QAbstractTableModel>
#include <QList>
#include <QMap>
#include <QStyledItemDelegate>
#include "label.h"
#include "savable.h"

// Combo-box editor for the label Type column.
class LabelTypeDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    void setEditorData(QWidget * editor, const QModelIndex & index) const override;
    void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;
};

class LabelList : public QAbstractTableModel, public Savable
{
    Q_OBJECT
public:
    explicit LabelList(QObject *parent = nullptr);

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    enum class Column {
        Type,
        Text,
        Last,
    };

    void addLabel(Label *l);
    bool removeLabel(Label *l, bool del = true);
    bool removeLabel(int index, bool del = true);
    Label *labelAt(int index) const;
    const QList<Label*> getLabels() const {return labels;}
    // Recomputes the resolved points of every label from the given symbol table.
    void reevaluateAll(const QMap<QString, double> &symbols);

    int rowCount(const QModelIndex &parent) const override { Q_UNUSED(parent) return labels.size();}
    int columnCount(const QModelIndex &parent) const override {Q_UNUSED(parent) return (int) Column::Last;}
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    int findIndex(Label *l);

    QList<Label*> labels;
};

#endif // LABELLIST_H
