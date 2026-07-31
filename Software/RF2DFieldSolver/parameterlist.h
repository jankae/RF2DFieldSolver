#ifndef PARAMETERLIST_H
#define PARAMETERLIST_H

#include <QAbstractTableModel>
#include <QList>
#include <QMap>
#include "savable.h"

// Ordered list of named parameters. Each parameter is an expression (float or
// equation) that may reference parameters defined earlier in the list. The
// evaluated results form a symbol table used both here and for the expression
// based vertices of the elements.
class ParameterList : public QAbstractTableModel, public Savable
{
    Q_OBJECT
public:
    explicit ParameterList(QObject *parent = nullptr);

    virtual nlohmann::json toJSON() override;
    virtual void fromJSON(nlohmann::json j) override;

    enum class Column {
        Name,
        Expression,
        Value,
        Last,
    };

    void addParameter(const QString &name = QString(), const QString &expression = QString());
    void removeParameter(int index);

    // Evaluated symbol table of all currently-valid parameters (name -> value).
    const QMap<QString, double>& symbols() const { return symbolCache; }

    int rowCount(const QModelIndex &parent) const override { Q_UNUSED(parent) return params.size(); }
    int columnCount(const QModelIndex &parent) const override { Q_UNUSED(parent) return (int) Column::Last; }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

signals:
    // Emitted whenever a parameter is added, removed or edited, i.e. whenever the
    // symbol table may have changed and dependent geometry must be recomputed.
    void parametersChanged();

private:
    struct Parameter {
        QString name;
        QString expression;
        double value;
        bool valid;
        QString error;
    };
    // Recomputes every parameter value in order and rebuilds the symbol table.
    void reevaluate();
    QString uniqueName() const;

    QList<Parameter> params;
    QMap<QString, double> symbolCache;
};

#endif // PARAMETERLIST_H
