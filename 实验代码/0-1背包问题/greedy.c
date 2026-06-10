#include "greedy.h"
#include "data_generator.h"

#include <stdlib.h>

typedef struct {
    int index;
    double ratio;
} ItemRatio;

static int compare_ratio_desc(const void *a, const void *b) {
    const ItemRatio *x = (const ItemRatio *)a;
    const ItemRatio *y = (const ItemRatio *)b;
    if (x->ratio > y->ratio) return -1;
    if (x->ratio < y->ratio) return 1;
    return 0;
}

static int compare_int_asc(const void *a, const void *b) {
    return (*(const int *)a) - (*(const int *)b);
}

KnapsackResult greedy_solve(const KnapsackData *data, int capacity) {
    KnapsackResult result;
    ItemRatio *items = NULL;
    int n = data->n;
    double remaining = (double)capacity;
    int i, count = 0;
    double t0, t1;

    knapsack_result_init(&result);
    result.optimal = 0;

    items = (ItemRatio *)malloc((size_t)n * sizeof(ItemRatio));
    if (!items) {
        knapsack_result_set_skipped(&result, "内存分配失败");
        return result;
    }

    result.space_bytes = (size_t)n * (sizeof(ItemRatio) + sizeof(int));
    t0 = get_time_ms();

    for (i = 0; i < n; i++) {
        items[i].index = i + 1;
        items[i].ratio = data->values[i] / data->weights[i];
    }
    qsort(items, (size_t)n, sizeof(ItemRatio), compare_ratio_desc);

    result.selected_items = (int *)malloc((size_t)n * sizeof(int));
    if (!result.selected_items) {
        free(items);
        knapsack_result_set_skipped(&result, "内存分配失败");
        return result;
    }

    for (i = 0; i < n; i++) {
        int idx = items[i].index;
        double w = data->weights[idx - 1];
        if (w <= remaining) {
            result.selected_items[count++] = idx;
            remaining -= w;
        }
    }

    result.num_selected = count;
    qsort(result.selected_items, (size_t)count, sizeof(int), compare_int_asc);

    compute_totals(data, result.selected_items, result.num_selected,
                   &result.total_weight, &result.total_value);
    result.total_value = round2(result.total_value);

    t1 = get_time_ms();
    result.elapsed_ms = t1 - t0;

    free(items);
    return result;
}
