/* EEG parsing application for 5SIA0
 *
 * Code by Glenn Bergmans (g.bergmans@student.tue.nl)
 * Code is based on various sources, most notably
 * the TU/e ES group code base and a Matlab
 * implementation by Mohammad Tahghighi
 */

#include "eeg.h"

int32_t randint(int32_t vmin, int32_t vmax)
{
    return (vmin + (int32_t) (rand() / (RAND_MAX / ((uint32_t) (vmax - vmin + 1)) + 1)));
}

int main(int argc, char *argv[]) {
    float features[CHANNELS][FEATURE_LENGTH];
    float favg[FEATURE_LENGTH] = {0};
	int32_t x[CHANNELS][DATAPOINTS];
	uint32_t i, j;

    // void open_set_num_threads(NUM_THREADS);
    read_data(x, CHANNELS, DATAPOINTS);
	omp_set_num_threads(NUM_THREADS);
     #pragma omp parallel for 
    for (i = 0; i < CHANNELS; i++) {
        printf("Running channel %d...\n", i);
        run_channel(DATAPOINTS, x[i], features[i]);
    }
	

    // Averaging channels
     #pragma omp parallel for private(i)
 for (j = 0; j < FEATURE_LENGTH; j++) {
    for (i = 0; i < CHANNELS; i++) {
            favg[j] += features[i][j] / FEATURE_LENGTH;
        }
    }

    printf("\n");
	for (i=0; i<FEATURE_LENGTH; i++)
        printf("Feature %d: %.6f\n", i, favg[i]);

    return 0;
}

void read_data(int32_t x[CHANNELS][DATAPOINTS], int nc, int np)
{
    FILE *fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int l, c;

    fp = fopen("EEG.csv", "r");
    if (fp == NULL) {
        printf("Error opening EEG.csv\n");
        exit(EXIT_FAILURE);
    }

    // Skip the first line
    getline(&line, &len, fp);

    l = 0;
    while ((l < np) && (read = getline(&line, &len, fp)) != -1) {
        char *tok;
        tok = strtok(line, ",");
        float v;

        for (c = 0; c < nc; c++) {
            sscanf(tok, "%f", &v);
            x[c][l] = (int32_t) round(v);
            tok = strtok(NULL, ",");
        }

        l++;
    }

}

void run_channel(int np, int32_t *x, float *features)
{
    // Butterworth returns np + 1 samples
	int32_t *X = malloc((np + 1) * sizeof(int32_t));

    // Clean signal using butterworth
    printf("    Butterworth filter...\n");
    bw0_int(np, x, X);

    // 4 features: mean, std dev, abs sum, mean crossings
    printf("    Standard features...\n");
    stafeature(np, X, &features[0]);
    // 2 features: mean p2p, std dev p2p
    printf("    Peak 2 peak features...\n");
    p2p(np, X, &features[4], 7);
    // 1 feature: aproximate entropy
    printf("    Aproximate Entropy feature...\n");
    apen(np, X, &features[6], 3, 0.2);
    // 1 feature: hurst coefficient
    printf("    Hurst Coefficient feature...\n");
    hurst(np, X, &features[7]);
    // 6 features: power in 5 frequency bands & total power
    printf("    Power Spectral Density features...\n");
    power_per_band(np, X, &features[8]);

    printf("Channel done\n");
    free(X);
}
