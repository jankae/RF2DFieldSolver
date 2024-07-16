#ifndef ELEMENTLIST_H
#define ELEMENTLIST_H

#include <QAbstractTableModel>
#include <QList>
#include <QStyledItemDelegate>
#include "element.h"

class TypeDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    //QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    void setEditorData(QWidget * editor, const QModelIndex & index) const override;
    void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;
};

class ElementList : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit ElementList(QObject *parent = nullptr);

    enum class Column {
        Name,
        Type,
        EpsilonR,
        Last,
    };

    void addElement(Element *e);
    bool removeElement(Element *e, bool del = true);
    bool removeElement(int index, bool del = true);
    Element *elementAt(int index) const;
    const QList<Element*> getElements() const {return elements;}

    int rowCount(const QModelIndex &parent) const override { Q_UNUSED(parent) return elements.size();}
    int columnCount(const QModelIndex &parent) const override {Q_UNUSED(parent) return (int) Column::Last;}
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:

    int findIndex(Element *e);

    QList<Element*> elements;
};

#endif // ELEMENTMODEL_H
