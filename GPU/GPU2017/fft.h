#ifndef FFT_H
#define FFT_H
/*----------------------------------------------------------------------------
   fft.c - fast Fourier transform and its inverse (both recursively)
   Copyright (C) 2004, Jerome R. Breitenbach.  All rights reserved.

   The author gives permission to anyone to freely copy, distribute, and use
   this file, under the following conditions:
      - No changes are made.
      - No direct commercial advantage is obtained.
      - No liability is attributed to the author for any damages incurred.
  ----------------------------------------------------------------------------*/

/******************************************************************************
 * This file defines a C function fft that, by calling another function       *
 * fft_rec (also defined), calculates an FFT recursively.  Usage:             *
 *   fft(N, x, X);                                                            *
 * Parameters:                                                                *
 *   N: number of points in FFT (must equal 2^n for some integer n >= 1)      *
 *   x: pointer to N time-domain samples given in rectangular form (Re x,     *
 *      Im x)                                                                 *
 *   X: pointer to N frequency-domain samples calculated in rectangular form  *
 *      (Re X, Im X)                                                          *
 * Similarly, a function ifft with the same parameters is defined that        *
 * calculates an inverse FFT (IFFT) recursively.  Usage:                      *
 *   ifft(N, x, X);                                                           *
 * Here, N and X are given, and x is calculated.                              *
 ******************************************************************************/

#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

void fft(int N, float (*x)[2], float (*X)[2]);

// Following functions by Glenn Bergmans (g.bergmans@student.tue.nl)
float power(float re, float im);
void power_per_band(int N, int32_t (*x), float *p);

#endif
