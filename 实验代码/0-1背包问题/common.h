#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>

#define MAX_SKIP_REASON 256
#define MAX_ITEMS_DISPLAY 20

typedef struct {
    int *selected_items;
    int num_selected;
    double total_weight;
    double total_value;
    double elapsed_ms;
    size_t space_bytes;
    int optimal;   /* 1=最优算法, 0=贪心 */
    int skipped;
    char skip_reason[MAX_SKIP_REASON];
} KnapsackResult;

typedef struct {
    int n;
    double *weights;
    double *values;
} KnapsackData;

void knapsack_result_init(KnapsackResult *r);
void knapsack_result_free(KnapsackResult *r);
void knapsack_result_set_skipped(KnapsackResult *r, const char *reason);
void knapsack_data_free(KnapsackData *data);

void compute_totals(const KnapsackData *data, const int *selected, int num_selected,
                    double *total_weight, double *total_value);

double get_time_ms(void);
int weight_for_dp(double weight);

void print_result_summary(const char *algo_name, const KnapsackResult *r);

#endif
