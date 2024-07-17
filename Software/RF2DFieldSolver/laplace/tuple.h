#ifndef INCLUDE_TUPLE_H
#define INCLUDE_TUPLE_H

#include <stdint.h>

struct rect {
    double x;
    double y;
};

struct point {
    uint32_t x;
    uint32_t y;
};

struct config {
    uint8_t threads;
    uint8_t distance;
    double threshold;
};

#endif
