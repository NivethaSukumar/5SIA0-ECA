#ifndef EEG_H
#define EEG_H

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "bw0_int.h"
#include "stafeature.h"
#include "p2p.h"
#include "apen.h"
#include "hurst.h"
#include "fft.h"

#define CHANNELS 23
#define DATAPOINTS 256
#define FEATURE_LENGTH 14
#define FS 100

//Uncomment for verbose messaging
//#define VERBOSE

//Uncomment for a CPU only program
//(NOTE: you have to insert the appropriate macros if you write your own gpu code of course, see stafeatures.cu for an example)
//#define CPU_ONLY

//Uncomment to enable debug printing (in original code printing in the gpu_average function)
//#define DEBUG

void read_data(int32_t x[CHANNELS][DATAPOINTS], int nc, int np);
void run_channel(int np, int32_t *x, float *features);


#ifdef __NVCC__
//Next section only relevant for the cuda compiler (nvcc)

//Helper definition to easily insert cuda error code checking
#define cudaCheckError(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true){
   if (code != cudaSuccess){
      fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
      if (abort) exit(code);
   }
}
#endif

#endif
