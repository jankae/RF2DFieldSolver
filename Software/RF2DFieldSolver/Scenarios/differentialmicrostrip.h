#ifndef DIFFERENTIALMICROSTRIP_H
#define DIFFERENTIALMICROSTRIP_H

#include "scenario.h"

class DifferentialMicrostrip : public Scenario
{
public:
    DifferentialMicrostrip();
protected:
    virtual ElementList *createScenario() override;
    virtual QPixmap getImage() override;
private:
    double width;
    double height;
    double gap;
    double substrate_height;
    double e_r;
};

#endif // DIFFERENTIALMICROSTRIP_H
