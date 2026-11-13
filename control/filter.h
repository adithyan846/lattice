#ifndef FILTER_H
#define FILTER_H

#include "fix16.h"

typedef struct {
    fix16_t state;
    fix16_t k;
} pt1_t;

void    pt1_init(pt1_t *f, fix16_t cutoff_hz, fix16_t dt);
void    pt1_set_cutoff(pt1_t *f, fix16_t cutoff_hz, fix16_t dt);
fix16_t pt1_apply(pt1_t *f, fix16_t input);

#endif
