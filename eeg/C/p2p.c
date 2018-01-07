#include "eeg.h"

int32_t min(int32_t a, int32_t b)
{
    return ((a < b) ? a : b);
}

int32_t max(int32_t a, int32_t b)
{
    return ((a > b) ? a : b);
}

// Finds all peaks, ignoring smaller peaks in a range
// Returns mean and stddev of peaks
// Based on code by: Mohammad Tahghighi
void p2p(int np, int32_t *x, float *pp, int mindist)
{
    int32_t *pospeak = malloc(np * sizeof(int32_t));
    int poscount = 0;
    int32_t *negpeak = malloc(np * sizeof(int32_t));
    int negcount = 0;
    int32_t *peak2peak = malloc(np/2 * sizeof(int32_t));
    int peak2peakcount;
    int i, j;
    bool dir_is_up;

    dir_is_up = x[1] > x[0];
    for (i = 0; i < np-1; i++) {
        if (dir_is_up && x[i + 1] < x[i]) {
            // This could be a positive peak
            bool is_peak = true;
            for (j = max(i - mindist, 0); j < min(i + mindist, np); j++) {
                if (x[j] > x[i]) {
                    // Not the highest peak in the area
                    is_peak = false;
                    break;
                }
            }
            if (is_peak) {
                pospeak[poscount] = x[i];
                poscount++;
                dir_is_up = false;
            }
        } else if (!dir_is_up && x[i + 1] > x[i]) {
            // This could be a negative peak
            bool is_peak = true;
            for (j = max(i - mindist, 0); j < min(i + mindist, np); j++) {
                if (x[j] < x[i]) {
                    // Not the lowest peak in the area
                    is_peak = false;
                    break;
                }
            }
            if (is_peak) {
                negpeak[negcount] = x[i];
                negcount++;
                dir_is_up = true;
            }
        }
    }

    // Calculate p2p value (and sum)
    peak2peakcount = min(poscount, negcount);
    for (i = 0; i < peak2peakcount; i++) {
        peak2peak[i] = abs(pospeak[i] - negpeak[i]);
    }

    // Calculate mean
    pp[0] = average(peak2peakcount, peak2peak);
    pp[1] = stddev(peak2peakcount, peak2peak, pp[0]);

    free(pospeak);
    free(negpeak);
    free(peak2peak);
}
