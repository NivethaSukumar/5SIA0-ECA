#include "eeg.h"

/* macros */
#define TWO_PI (6.2831853071795864769252867665590057683943L)

//prototype
void fft_rec(int N, int offset, int delta, float (*x)[2], float (*X)[2], float (*XX)[2]);

void fft(int N, float (*x)[2], float (*X)[2])
{
    /* Declare a pointer to scratch space. */
    float (*XX)[2] = (float(*)[2]) malloc(2 * N * sizeof(float));

    /* Calculate FFT by a recursion. */
    fft_rec(N, 0, 1, x, X, XX);

    /* Free memory. */
    free(XX);
}

void fft_rec(int N, int offset, int delta,
        float (*x)[2], float (*X)[2], float (*XX)[2])
{
    int N2 = N/2;            /* half the number of points in FFT */
    int k;                   /* generic index */
    float cs, sn;           /* cosine and sine */
    int k00, k01, k10, k11;  /* indices for butterflies */
    float tmp0, tmp1;       /* temporary storage */

    if(N != 2)  /* Perform recursive step. */
    {
        /* Calculate two (N/2)-point DFT's. */
        fft_rec(N2, offset, 2*delta, x, XX, X);
        fft_rec(N2, offset+delta, 2*delta, x, XX, X);

        /* Combine the two (N/2)-point DFT's into one N-point DFT. */
        for(k=0; k<N2; k++)
        {
            k00 = offset + k*delta;    k01 = k00 + N2*delta;
            k10 = offset + 2*k*delta;  k11 = k10 + delta;
            cs = cos(TWO_PI*k/(float)N); sn = sin(TWO_PI*k/(float)N);
            tmp0 = cs * XX[k11][0] + sn * XX[k11][1];
            tmp1 = cs * XX[k11][1] - sn * XX[k11][0];
            X[k01][0] = XX[k10][0] - tmp0;
            X[k01][1] = XX[k10][1] - tmp1;
            X[k00][0] = XX[k10][0] + tmp0;
            X[k00][1] = XX[k10][1] + tmp1;
        }
    }
    else  /* Perform 2-point DFT. */
    {
        k00 = offset; k01 = k00 + delta;
        X[k01][0] = x[k00][0] - x[k01][0];
        X[k01][1] = x[k00][1] - x[k01][1];
        X[k00][0] = x[k00][0] + x[k01][0];
        X[k00][1] = x[k00][1] + x[k01][1];
    }
}

float power(float re, float im)
{
    return re * re + im * im;
}

void power_per_band(int32_t N, int32_t (*x), float *p)
{
    const int BANDS = 5;
    float (*xx)[2] = (float(*)[2])  malloc(2 * N * sizeof(float));
    float (*X)[2] = (float (*)[2]) malloc(2 * N * sizeof(float));
    int i, j;
    int bands[BANDS + 1];

    // Delta: <= 4 Hz
    bands[0] = (4 * N) / FS;
    // Theta:  4 < f <= 7 Hz
    bands[1] = (7 * N) / FS;
    // Alpha: 7 < f <= 15 Hz
    bands[2] = (15 * N) / FS;
    // Beta:   15 < f <= 31 Hz
    bands[3] = (31 * N) / FS;
    // Gamma: > 31 Hz
    bands[4] = N;

    for (i = 0; i < N; i++) {
        xx[i][0] = (float) x[i];
        xx[i][1] = 0.0f;
    }

    fft(N, xx, X);

    // Calcuclate power per band
    // Last item (p[BANDS]) is total power
    float pb = 0;
    for (i = 0, j = 0; i < BANDS; i++) {
        float pi = 0;
        for (; j < bands[i]; j++) {
            pi += power(X[j][0], X[j][1]);
        }
        pb += pi;
        p[i] = pi / ((float) (N * N));
    }
    p[BANDS] = pb / ((float) (N * N));
}

/*
int32_t randint(int32_t vmin, int32_t vmax)
{
    return (vmin + (int32_t) (rand() / (RAND_MAX / ((uint32_t) (vmax - vmin + 1)) + 1)));
}

#define DATAPOINTS 64
int main(int argc, char *argv[]) {
	float x[DATAPOINTS] = {0};
    //float X[DATAPOINTS][2] = {{0}};
    float p[BANDS + 1];
	uint32_t i;

    srand (time(NULL));
	for (i=0; i<DATAPOINTS; i++)
		x[i] = (float) randint(-100, 100);

	for (i=0; i<DATAPOINTS; i++)
        printf("%.0f\n", x[i]);
    printf("\n");

    //fft(DATAPOINTS, x, X);
    power_per_band(DATAPOINTS, x, p);

	for (i=0; i<BANDS + 1; i++)
        printf("power %2d: %.6f\n", i, p[i]);

	exit(0);
}*/
