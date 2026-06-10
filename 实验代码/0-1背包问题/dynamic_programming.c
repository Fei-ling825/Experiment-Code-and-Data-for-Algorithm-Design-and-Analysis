#include "dynamic_programming.h"
#include "data_generator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

KnapsackResult dp_solve(const KnapsackData *data, int capacity) {
    KnapsackResult result;
    int n = data->n;
    long long cells;
    double *dp_prev = NULL;
    double *dp_curr = NULL;
    char *take = NULL;
    int i, c, w, cap;
    double t0, t1;

    knapsack_result_init(&result);
    cells = (long long)n * (capacity + 1);

    dp_prev = (double *)calloc((size_t)(capacity + 1), sizeof(double));
    dp_curr = (double *)calloc((size_t)(capacity + 1), sizeof(double));
    take = (char *)calloc((size_t)n * (capacity + 1), sizeof(char));

    if (!dp_prev || !dp_curr || !take) {
        free(dp_prev);
        free(dp_curr);
        free(take);
        knapsack_result_set_skipped(&result, "内存分配失败");
        return result;
    }

    result.space_bytes = (size_t)(capacity + 1) * sizeof(double) * 2
                       + (size_t)cells * sizeof(char);

    t0 = get_time_ms();

    for (i = 1; i <= n; i++) {
        w = weight_for_dp(data->weights[i - 1]);
        memcpy(dp_curr, dp_prev, (size_t)(capacity + 1) * sizeof(double));

        for (c = 0; c <= capacity; c++) {
            if (w <= c) {
                double candidate = dp_prev[c - w] + data->values[i - 1];
                if (candidate > dp_curr[c]) {
                    dp_curr[c] = candidate;
                    take[(size_t)(i - 1) * (capacity + 1) + c] = 1;
                }
            }
        }

        {
            double *tmp = dp_prev;
            dp_prev = dp_curr;
            dp_curr = tmp;
        }
    }

    result.num_selected = 0;
    cap = capacity;
    for (i = n; i >= 1; i--) {
        if (take[(size_t)(i - 1) * (capacity + 1) + cap]) {
            result.num_selected++;
            cap -= weight_for_dp(data->weights[i - 1]);
        }
    }

    result.selected_items = (int *)malloc((size_t)result.num_selected * sizeof(int));
    if (!result.selected_items) {
        free(dp_prev);
        free(dp_curr);
        free(take);
        knapsack_result_set_skipped(&result, "内存分配失败");
        return result;
    }

    {
        int k = result.num_selected - 1;
        cap = capacity;
        for (i = n; i >= 1; i--) {
            if (take[(size_t)(i - 1) * (capacity + 1) + cap]) {
                result.selected_items[k--] = i;
                cap -= weight_for_dp(data->weights[i - 1]);
            }
        }
    }

    compute_totals(data, result.selected_items, result.num_selected,
                   &result.total_weight, &result.total_value);
    result.total_value = round2(result.total_value);

    t1 = get_time_ms();
    result.elapsed_ms = t1 - t0;
    result.optimal = 1;

    free(dp_prev);
    free(dp_curr);
    free(take);
    return result;
}
