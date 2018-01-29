#include "eeg.h"

// Code by Glenn Bergmans (g.bergmans@student.tue.nl)
// Based on code by Jos Huisken
// Algorithm based on: http://www.bearcave.com/misl/misl_tech/wavelets/hurst/

float avg_scaled_range(uint32_t np, int32_t *x, int32_t n)
{
    // Calculate average scaled region R/S for regions of size n

    int32_t M;
	int32_t *X =(int32_t *)  malloc(n * sizeof(int32_t));
	int32_t *Y =(int32_t *)  malloc(n * sizeof(int32_t));
    int32_t minY, maxY;
    int32_t V;
    float R, S;
    float totalRS = 0;
    int i;
    uint32_t r;

    for (r = 0; r < np; r += n) {
        // Calculate mean
        M = 0;
        for (i = 0; i<n; i++) {
            M += x[r + i];
        }
        M /= n;

        // Calculate X[i] = x[i] - M
        // Calculate Y[i] = sum(X[k] for k = 0 to i) = Y[i-1] + X[i]
        X[0] = x[r] - M;
        Y[0] = X[0];
        for (i = 1; i<n; i++) {
            X[i] = x[r + i] - M;
            Y[i] = Y[i - 1] + X[i];
        }

        // Calculate range R = max(Y) - min(Y)
        minY = maxY = Y[0];
        for (i = 1; i<n; i++) {
            if (Y[i] > maxY)
                maxY = Y[i];
            if (Y[i] < minY)
                minY = Y[i];
        }
        R = (float) (maxY - minY);

        // Calculate S = stddev(x) = sqrt(variance(x));
        // variance(x) = sum((x - M)^2) / n = sum(X^2) / n
        V = 0;
        for (i = 0; i<n; i++) {
            V += X[i] * X[i];
        }
        S = sqrtf((float) V / (float) n);

        totalRS += (R / S);
    }

    // Average all R/S values
    return (totalRS / ((float) (np / n)));
}

void hurst(uint32_t np, int32_t *x, float *h)
{
    // Divide input recursively into regions of minimal size of 8
	assert(np >= 8); // Min region size is 8
    assert(!(np & (np - 1))); // Make sure is power of two

    int32_t log2np = (int32_t) log2(np);
    int32_t steps = log2np - 2;
	float *graphx = (float *) malloc(steps * sizeof(float));
	float *graphy = (float *) malloc(steps * sizeof(float));
    const float logtwo = log(2);
	float H;
    int i;

	float Sumx, Sumy, Sumxy, Sumxx;

    for (i = 0; i < steps; i++) {
        int32_t region_size = np / (1 << i); // np, np / 2, ..., 512, 256, ..., 8
        graphx[i] = log2np - i; // == log2(region_size)
        graphy[i] = log(avg_scaled_range(np, x, region_size)) / logtwo;
    }

    // linear fitting of the plotted points at (x,y) = (log2(region_size), log2(scaled_region))
	Sumx = 0;
	Sumy = 0;
	Sumxy = 0;
	Sumxx = 0;
	for (i = 0; i < steps; i++) {
		Sumx = Sumx + graphx[i];
		Sumy = Sumy + graphy[i];
		Sumxy = Sumxy + (graphx[i]) * (graphy[i]);
		Sumxx = Sumxx + (graphx[i]) * (graphx[i]);
	}
	/* printf("Log(time) Log(R/S)\n"); */
	/* for (i = 0; i < NoOfPlottedPoints; i++) */
	/* 	printf("%g\t%g\n", graphx[i], graphy[i]); */
	//Calculate Hurst coefficient
	H = (Sumxy - ((Sumx * Sumy) / steps)) / (Sumxx - ((Sumx * Sumx) / steps));
	//printf("\nHurst = %g\n", H);
	*h = H;
}
