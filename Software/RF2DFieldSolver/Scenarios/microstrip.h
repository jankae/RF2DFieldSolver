#ifndef MICROSTRIP_H
#define MICROSTRIP_H

#include "scenario.h"

class Microstrip : public Scenario
{
public:
    Microstrip();
protected:
    virtual ElementList *createScenario() override;
private:
    double width;
    double height;
    double substrate_height;
    double e_r;
};

#endif // MICROSTRIP_H
