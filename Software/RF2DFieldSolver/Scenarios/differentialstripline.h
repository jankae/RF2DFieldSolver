#ifndef DIFFERENTIALSTRIPLINE_H
#define DIFFERENTIALSTRIPLINE_H

#include "scenario.h"

class DifferentialStripline : public Scenario
{
public:
    DifferentialStripline();
protected:
    virtual ElementList *createScenario() override;
    virtual QPixmap getImage() override;
private:
    double width;
    double height;
    double gap;
    double substrate_height_above;
    double e_r_above;
    double substrate_height_below;
    double e_r_below;
};

#endif // DIFFERENTIALSTRIPLINE_H
