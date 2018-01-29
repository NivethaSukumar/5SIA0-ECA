#ifndef STAFEATURE_H
#define STAFEATURE_H

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

int32_t abssum(int np, int32_t *x);
float average(int np, int32_t *x);
float variance(int np, int32_t *x, float avg);
float stddev(int np, int32_t *x, float avg);
int mean_crosstimes(int np, int32_t *x, float avg);
void stafeature(int np, int32_t *x, float *sta);

#endif
