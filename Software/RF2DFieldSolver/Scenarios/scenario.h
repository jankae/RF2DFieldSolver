#ifndef SCENARIO_H
#define SCENARIO_H

#include <QDialog>
#include <QList>
#include <QRectF>

#include "elementlist.h"

namespace Ui {
class Scenario;
}

class Scenario : public QDialog
{
    Q_OBJECT

public:
    explicit Scenario(QWidget *parent = nullptr);
    ~Scenario();

    static QList<Scenario*> createAll();

    void setupParameters();
    const QString &getName() const { return name; }

signals:
    void scenarioCreated(QPointF topLeft, QPointF bottomRight, ElementList *list);

protected:
    virtual ElementList *createScenario() = 0;
    virtual QPixmap getImage() = 0;

    using Parameter = struct {
        QString name;
        QString unit;
        QString prefixes;
        int precision;
        double *value;
    };
    QList<Parameter> parameters;
    QString name;
    Ui::Scenario *ui;
private:
    };
#endif // SCENARIO_H
