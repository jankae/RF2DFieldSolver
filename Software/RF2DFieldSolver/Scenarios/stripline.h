#ifndef STRIPLINE_H
#define STRIPLINE_H

#include "scenario.h"

class Stripline : public Scenario
{
public:
    Stripline();
protected:
    virtual ElementList *createScenario() override;
private:
    double width;
    double height;
    double substrate_height_above;
    double e_r_above;
    double substrate_height_below;
    double e_r_below;
};

#endif // STRIPLINE_H
