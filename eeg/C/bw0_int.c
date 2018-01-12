#include "eeg.h"

// Code by: Jos Huisken

// Fixed point coefficients
static const int32_t a[order+1] = {
	1048576, -959659, -3650400, 2700937, 5580616,
	-3022504, -4541085, 1561834, 1933496, -311478, -340338 };
static const int32_t b[order+1] = {
	597385, 0, -2986929, 0, 5973856, 0, -5973857, 0,
	2986928, 0, -597386 };

#define SCALE (b[0]*16)  // *16??

void bw0_int(int np, int32_t *x, int32_t *y)
{
    int i, j, k;
    int64_t yy;
    yy=b[0] * x[0];
    y[0] = yy / SCALE;
		#pragma omp parallel for private(j,k)
    for (i=1;i<order+1;i++)
    {
        yy=0;
				//#pragma omp parallel for reduction(+:yy)
        for (j=0;j<i+1;j++)
            yy=yy+b[j]*x[i-j];
				//#pragma omp parallel for reduction(-:yy)
        for (k=0;k<i;k++)
            yy=yy-a[k+1]*y[i-k-1];
	y[i] = yy / SCALE;
    }
		#pragma omp parallel for private(j,k)
    for (i=order+1;i<np+1;i++)
    {
        yy=0;
				//#pragma omp parallel for reduction(+:yy)
        for (j=0;j<order+1;j++)
            yy=yy+b[j]*x[i-j];
			  //#pragma omp parallel for reduction(-:yy)
        for (k=0;k<order;k++)
            yy=yy-a[k+1]*y[i-k-1];
	y[i] = yy / SCALE;
    }
}

/*void bw0_int(int np, int32_t *x, int32_t *y)
{
    int i, j;
    int64_t yy;
    yy=b[0] * x[0];
    y[0] = yy / SCALE;
    for (i=1;i<order+1;i++)
    {
        yy=0;
        for (j=0;j<i+1;j++)
            yy=yy+b[j]*x[i-j];
        for (j=0;j<i;j++)
            yy=yy-a[j+1]*y[i-j-1];
	y[i] = yy / SCALE;
    }
    for (i=order+1;i<np+1;i++)
    {
        yy=0;
        for (j=0;j<order+1;j++)
            yy=yy+b[j]*x[i-j];
        for (j=0;j<order;j++)
            yy=yy-a[j+1]*y[i-j-1];
	y[i] = yy / SCALE;
    }
}*/
