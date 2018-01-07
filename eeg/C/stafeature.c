#include "eeg.h"

// Based on code by: Mohammad Tahghighi
int32_t abssum(int np, int32_t *x)
{
    int i;
    int32_t s = 0;

    for (i = 0; i < np; i++) {
        s += abs(x[i]);
    }

    return s;
}

float average(int np, int32_t *x)
{
    int i;
    int32_t s = 0;

    for (i = 0; i < np; i++) {
        s += x[i];
    }

    return ((float) s) / ((float) np);
}

float variance(int np, int32_t *x, float avg)
{
    int i;
    float s = 0;

    // Variance = Sum((x - avg)^2)
    for (i = 0; i < np; i++) {
        float tmp = x[i] - avg;
        s += (tmp * tmp);
    }

    return s / ((float) np);
}

float stddev(int np, int32_t *x, float avg)
{
    // Stddev = sqrt(variance)
    float var = variance(np, x, avg);
    return sqrt(var);
}

int mean_crosstimes(int np, int32_t *x, float avg)
{
    int i;
    bool negative = x[0] < avg;
    int count = 0;

    // Count number of zero crossings for (x - avg)
    for (i = 0; i < np; i++) {
        if (negative) {
            if (x[i] > avg) {
                negative = false;
                count++;
            }
        } else {
            if (x[i] < avg) {
                negative = true;
                count++;
            }
        }
    }

    return count;
}

void stafeature(int np, int32_t *x, float *sta)
{
    // Returns sta = [mean, std, abssum, mean_crosstimes)

    float avg = average(np, x);
    sta[0] = avg;

    sta[1] = stddev(np, x, avg);
    sta[2] = abssum(np, x);
    sta[3] = mean_crosstimes(np, x, avg);
}

/*
int32_t randint(int32_t vmin, int32_t vmax)
{
    return (vmin + (int32_t) (rand() / (RAND_MAX / ((uint32_t) (vmax - vmin + 1)) + 1)));
}

#define DATAPOINTS 32
int main(int argc, char *argv[]) {
    int32_t sta[4] = {0};
	int32_t x[DATAPOINTS];
	uint32_t i;

    srand (time(NULL));
	for (i=0; i<DATAPOINTS; i++)
		x[i] = randint(-100, 100);

    stafeature(DATAPOINTS, x, sta);
    printf("Mean: %d\n", sta[0]);
    printf("Std dev: %d\n", sta[1]);
    printf("Abs Sum: %d\n", sta[2]);
    printf("Mean Crossings: %d\n", sta[3]);

	exit(0);
}*/
