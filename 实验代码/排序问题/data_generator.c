#include "data_generator.h"

#include <stdio.h>
#include <stdlib.h>

static unsigned int rng_state = 1;

static void rng_seed(unsigned int seed) {
    rng_state = seed ? seed : 1;
}

static int rng_int(int min_val, int max_val) {
    rng_state = rng_state * 1103515245u + 12345u;
    return min_val + (int)(rng_state % (unsigned int)(max_val - min_val + 1));
}

int *generate_random_data(int size, unsigned int seed) {
    int *data;
    int i;
    int upper;

    if (size <= 0) {
        return NULL;
    }

    data = (int *)malloc((size_t)size * sizeof(int));
    if (!data) {
        return NULL;
    }

    upper = size * 10 - 1;
    if (upper < 1) {
        upper = 1;
    }

    rng_seed(seed);
    for (i = 0; i < size; i++) {
        data[i] = rng_int(0, upper);
    }
    return data;
}

void print_data_preview(const int *data, int size) {
    int i;
    int show = size <= 20 ? size : 10;

    printf("[");
    for (i = 0; i < show; i++) {
        printf("%d%s", data[i], i + 1 < show ? ", " : "");
    }
    if (size > 20) {
        printf(", ... (%d 个元素) ..., ", size);
        for (i = size - 10; i < size; i++) {
            printf("%d%s", data[i], i + 1 < size ? ", " : "");
        }
    }
    printf("]\n");
}
