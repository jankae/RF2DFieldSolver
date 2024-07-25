#include "coplanardifferentialmicrostrip.h"

#include "ui_scenario.h"

CoplanarDifferentialMicrostrip::CoplanarDifferentialMicrostrip()
{
    name = "Coplanar Differential Microstrip";
    width = 0.3e-3;
    height = 35e-6;
    gapTrace = 0.2e-3;
    gapCoplanar = 0.3e-3;
    substrate_height = 0.2e-3;
    e_r = 4.1;
    // create parameters
    parameters.push_back({.name = "Trace Width (w)", .unit = "m", .prefixes = "um ", .precision = 4, .value = &width});
    parameters.push_back({.name = "Trace Height (t)", .unit = "m", .prefixes = "um ", .precision = 4, .value = &height});
    parameters.push_back({.name = "Gap Width (s1)", .unit = "m", .prefixes = "um ", .precision = 4, .value = &gapTrace});
    parameters.push_back({.name = "Gap Width (s2)", .unit = "m", .prefixes = "um ", .precision = 4, .value = &gapCoplanar});
    parameters.push_back({.name = "Substrate Height (h)", .unit = "m", .prefixes = "um ", .precision = 4, .value = &substrate_height});
    parameters.push_back({.name = "Substrate dielectric constant", .unit = "", .prefixes = " ", .precision = 3, .value = &e_r});
}

ElementList *CoplanarDifferentialMicrostrip::createScenario()
{
    auto list = new ElementList();

    if(ui->autoArea->isChecked()) {
        ui->xleft->setValue(-std::max({substrate_height * 5, (width+gapTrace/2) * 3, gapCoplanar * 4}));
        ui->xright->setValue(std::max({substrate_height * 5, (width+gapTrace/2) * 3, gapCoplanar * 4}));
        ui->ytop->setValue(substrate_height * 5);
        ui->ybottom->setValue(-substrate_height-0.1e-3);
    }

    auto trace = new Element(Element::Type::TracePos);
    trace->appendVertex(QPointF(-width - gapTrace/2, 0));
    trace->appendVertex(QPointF(-gapTrace/2, 0));
    trace->appendVertex(QPointF(-gapTrace/2, height));
    trace->appendVertex(QPointF(-width - gapTrace/2, height));
    list->addElement(trace);

    auto trace2 = new Element(Element::Type::TraceNeg);
    trace2->appendVertex(QPointF(gapTrace/2, 0));
    trace2->appendVertex(QPointF(gapTrace/2 + width, 0));
    trace2->appendVertex(QPointF(gapTrace/2 + width, height));
    trace2->appendVertex(QPointF(gapTrace/2, height));
    list->addElement(trace2);

    // coplanar GNDs
    auto gnd1 = new Element(Element::Type::GND);
    gnd1->appendVertex(QPointF(ui->xleft->value(), 0));
    gnd1->appendVertex(QPointF(-gapTrace/2 - width - gapCoplanar, 0));
    gnd1->appendVertex(QPointF(-gapTrace/2 - width - gapCoplanar, height));
    gnd1->appendVertex(QPointF(ui->xleft->value(), height));
    list->addElement(gnd1);

    auto gnd2 = new Element(Element::Type::GND);
    gnd2->appendVertex(QPointF(gapTrace/2 + width + gapCoplanar, 0));
    gnd2->appendVertex(QPointF(ui->xright->value(), 0));
    gnd2->appendVertex(QPointF(ui->xright->value(), height));
    gnd2->appendVertex(QPointF(gapTrace/2 + width + gapCoplanar, height));
    list->addElement(gnd2);

    auto substrate = new Element(Element::Type::Dielectric);
    substrate->setEpsilonR(e_r);
    substrate->appendVertex(QPointF(ui->xleft->value(), 0));
    substrate->appendVertex(QPointF(ui->xright->value(), 0));
    substrate->appendVertex(QPointF(ui->xright->value(), -substrate_height));
    substrate->appendVertex(QPointF(ui->xleft->value(), -substrate_height));
    list->addElement(substrate);

    auto gnd = new Element(Element::Type::GND);
    gnd->appendVertex(QPointF(ui->xleft->value(), -substrate_height));
    gnd->appendVertex(QPointF(ui->xright->value(), -substrate_height));
    gnd->appendVertex(QPointF(ui->xright->value(), -substrate_height-0.1e-3));
    gnd->appendVertex(QPointF(ui->xleft->value(), -substrate_height-0.1e-3));
    list->addElement(gnd);

    return list;
}

QPixmap CoplanarDifferentialMicrostrip::getImage()
{
    return QPixmap(":/images/coplanar_microstrip_differential.png");
}

