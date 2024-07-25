#ifndef COPLANARMICROSTRIP_H
#define COPLANARMICROSTRIP_H

#include "scenario.h"

class CoplanarMicrostrip : public Scenario
{
public:
    CoplanarMicrostrip();
protected:
    virtual ElementList *createScenario() override;
    virtual QPixmap getImage() override;
private:
    double width;
    double gap;
    double height;
    double substrate_height;
    double e_r;
};

#endif // COPLANARMICROSTRIP_H
