#ifndef COPLANARDIFFERENTIALSTRIPLINE_H
#define COPLANARDIFFERENTIALSTRIPLINE_H

#include "scenario.h"

class CoplanarDifferentialStripline : public Scenario
{
public:
    CoplanarDifferentialStripline();
protected:
    virtual ElementList *createScenario() override;
    virtual QPixmap getImage() override;
private:
    double width;
    double height;
    double gapTrace;
    double gapCoplanar;
    double substrate_height_above;
    double e_r_above;
    double substrate_height_below;
    double e_r_below;
};

#endif // COPLANARDIFFERENTIALSTRIPLINE_H
