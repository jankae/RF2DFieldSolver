#include "microstrip.h"

#include "ui_scenario.h"

Microstrip::Microstrip()
{
    name = "Microstrip";
    width = 0.3e-3;
    height = 35e-6;
    substrate_height = 0.2e-3;
    e_r = 4.1;
    // create parameters
    parameters.push_back({.name = "Trace Width (w)", .unit = "m", .prefixes = "um ", .precision = 4, .value = &width});
    parameters.push_back({.name = "Trace Height (t)", .unit = "m", .prefixes = "um ", .precision = 4, .value = &height});
    parameters.push_back({.name = "Substrate Height (h)", .unit = "m", .prefixes = "um ", .precision = 4, .value = &substrate_height});
    parameters.push_back({.name = "Substrate dielectric constant", .unit = "", .prefixes = " ", .precision = 3, .value = &e_r});
}

ElementList *Microstrip::createScenario()
{
    auto list = new ElementList();

    if(ui->autoArea->isChecked()) {
        ui->xleft->setValue(-std::max(substrate_height * 5, width));
        ui->xright->setValue(std::max(substrate_height * 5, width));
        ui->ytop->setValue(substrate_height * 5);
        ui->ybottom->setValue(-substrate_height-0.1e-3);
    }

    auto trace = new Element(Element::Type::TracePos);
    trace->appendVertex(QPointF(-width/2, 0));
    trace->appendVertex(QPointF(width/2, 0));
    trace->appendVertex(QPointF(width/2, height));
    trace->appendVertex(QPointF(-width/2, height));
    list->addElement(trace);

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

