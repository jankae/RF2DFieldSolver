#include "scenario.h"
#include "ui_scenario.h"

#include "CustomWidgets/siunitedit.h"

#include "microstrip.h"
#include "stripline.h"
#include "coplanarmicrostrip.h"
#include "coplanarstripline.h"
#include "differentialmicrostrip.h"
#include "coplanardifferentialmicrostrip.h"
#include "differentialstripline.h"
#include "coplanardifferentialstripline.h"

Scenario::Scenario(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Scenario)
{
    ui->setupUi(this);

    ui->parameters->setLayout(new QFormLayout);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &Scenario::reject);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [=](){
        // update parameters
        for(unsigned int i=0;i<parameters.size();i++) {
            auto layout = static_cast<QFormLayout*>(ui->parameters->layout());
            auto entry = static_cast<SIUnitEdit*>(layout->itemAt(i, QFormLayout::FieldRole)->widget());
            *parameters[i].value = entry->value();
        }

        auto list = createScenario();
        emit scenarioCreated(QPointF(ui->xleft->value(), ui->ytop->value()), QPointF(ui->xright->value(), ui->ybottom->value()), list);
        accept();
    });
    ui->autoArea->setChecked(true);
    ui->xleft->setUnit("m");
    ui->xleft->setPrefixes("um ");
    ui->xleft->setPrecision(4);
    ui->xleft->setValue(-3e-3);

    ui->xright->setUnit("m");
    ui->xright->setPrefixes("um ");
    ui->xright->setPrecision(4);
    ui->xright->setValue(3e-3);

    ui->ytop->setUnit("m");
    ui->ytop->setPrefixes("um ");
    ui->ytop->setPrecision(4);
    ui->ytop->setValue(3e-3);

    ui->ybottom->setUnit("m");
    ui->ybottom->setPrefixes("um ");
    ui->ybottom->setPrecision(4);
    ui->ybottom->setValue(-1e-3);
}

Scenario::~Scenario()
{
    delete ui;
}

QList<Scenario *> Scenario::createAll()
{
    QList<Scenario*> ret;
    ret.push_back(new Microstrip());
    ret.push_back(new CoplanarMicrostrip());
    ret.push_back(new DifferentialMicrostrip());
    ret.push_back(new CoplanarDifferentialMicrostrip());
    ret.push_back(new Stripline());
    ret.push_back(new CoplanarStripline());
    ret.push_back(new DifferentialStripline());
    ret.push_back(new CoplanarDifferentialStripline());

    for(auto s : ret) {
        s->setupParameters();
    }
    return ret;
}

void Scenario::setupParameters()
{
    auto layout = static_cast<QFormLayout*>(ui->parameters->layout());
    for(auto &p : parameters) {
        auto label = new QLabel(p.name+":");
        auto entry = new SIUnitEdit(p.unit, p.prefixes, p.precision);
        entry->setValue(*p.value);
        layout->addRow(label, entry);
    }
    setWindowTitle(name + " Setup Dialog");

    // show the image
    ui->image->setPixmap(getImage());
}
