#ifndef DATA_GENERATOR_H
#define DATA_GENERATOR_H

#include "common.h"

#define TABLE1_ITEM_COUNT 1000
#define TABLE1_SEED 20261000U

void rng_seed(unsigned int seed);
double rng_uniform(double min_val, double max_val);
int rng_int(int min_val, int max_val);
double round2(double x);

int generate_knapsack_data(KnapsackData *data, int n_items, unsigned int seed, int decimal_weights);
int get_table1_data(KnapsackData *data);

#endif
