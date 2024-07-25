#ifndef COPLANARDIFFERENTIALMICROSTRIP_H
#define COPLANARDIFFERENTIALMICROSTRIP_H

#include "scenario.h"

class CoplanarDifferentialMicrostrip : public Scenario
{
public:
    CoplanarDifferentialMicrostrip();
protected:
    virtual ElementList *createScenario() override;
    virtual QPixmap getImage() override;
private:
    double width;
    double height;
    double gapTrace;
    double gapCoplanar;
    double substrate_height;
    double e_r;
};

#endif // COPLANARDIFFERENTIALMICROSTRIP_H
