#include "data_generator.h"

#include <stdlib.h>

static unsigned int rng_state = 1;

void rng_seed(unsigned int seed) {
    rng_state = seed ? seed : 1;
}

double rng_uniform(double min_val, double max_val) {
    rng_state = rng_state * 1103515245u + 12345u;
    double t = (rng_state % 1000000) / 1000000.0;
    return min_val + t * (max_val - min_val);
}

int rng_int(int min_val, int max_val) {
    rng_state = rng_state * 1103515245u + 12345u;
    return min_val + (int)(rng_state % (unsigned int)(max_val - min_val + 1));
}

double round2(double x) {
    return ((int)(x * 100.0 + 0.5)) / 100.0;
}

int generate_knapsack_data(KnapsackData *data, int n_items, unsigned int seed, int decimal_weights) {
    int i;
    (void)decimal_weights;
    data->n = n_items;
    data->weights = (double *)malloc((size_t)n_items * sizeof(double));
    data->values = (double *)malloc((size_t)n_items * sizeof(double));
    if (!data->weights || !data->values) {
        knapsack_data_free(data);
        return 0;
    }

    rng_seed(seed);
    for (i = 0; i < n_items; i++) {
        /* 题目要求：重量 1~100 随机整数，价值 100~1000 保留两位小数 */
        data->weights[i] = (double)rng_int(1, 100);
        data->values[i] = round2(rng_uniform(100.0, 1000.0));
    }
    return 1;
}

int get_table1_data(KnapsackData *data) {
    return generate_knapsack_data(data, TABLE1_ITEM_COUNT, TABLE1_SEED, 0);
}
