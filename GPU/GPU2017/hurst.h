#ifndef HURST_H
#define HURST_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

void hurst(uint32_t NoOfDataPoints, int32_t *data, float *h);

#endif
