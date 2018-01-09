#include "eeg.h"

double apen_correlation (int np, int32_t *x, unsigned int m, double r)
{
    bool set;
    unsigned int count;
    double sum = 0;

    for (unsigned int i = 0; i <= np - (m + 1) + 1; i++) {
        count = 0;
        for (unsigned int j = 0; j <= np - (m + 1) + 1; j++) {
            set = false;

            for (unsigned int k = 0; k < m; k++) {
                if (abs(x[i + k] - x[j + k]) > r) {
                    set = true;
                    break;
                }
            }
            if (set == false) count++;
        }
        sum += ((double) count) / ((double) np - m + 1);
    }

    return sum / ((double) np - m + 1);
}

void apen(int np, int32_t *x, float *a, unsigned int m, double r)
{
    // Based on: https://nl.mathworks.com/matlabcentral/fileexchange/26546-approximate-entropy
    float A;

    A = log(apen_correlation(np, x, m, r) / apen_correlation(np, x, m + 1, r));

    //Convert to fixed point
    *a = A;
}
