#include "backtracking.h"
#include "data_generator.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int item;
    double ratio;
} OrderItem;

static int compare_order_desc(const void *a, const void *b) {
    const OrderItem *x = (const OrderItem *)a;
    const OrderItem *y = (const OrderItem *)b;
    if (x->ratio > y->ratio) return -1;
    if (x->ratio < y->ratio) return 1;
    return 0;
}

static int compare_int_asc(const void *a, const void *b) {
    return (*(const int *)a) - (*(const int *)b);
}

static double fractional_bound(int idx, double current_weight, double current_value,
                               int capacity, const KnapsackData *data, const int *order) {
    double bound = current_value;
    double w = current_weight;
    int n = data->n;
    int j;

    for (j = idx; j < n && w < capacity; j++) {
        int item = order[j];
        double wi = data->weights[item - 1];
        double vi = data->values[item - 1];
        if (w + wi <= capacity) {
            w += wi;
            bound += vi;
        } else {
            bound += vi * (capacity - w) / wi;
            break;
        }
    }
    return bound;
}

typedef struct {
    const KnapsackData *data;
    int capacity;
    const int *order;
    double best_value;
    int *best_selected;
    int best_count;
    int n;
} BtContext;

static void backtrack(BtContext *ctx, int idx, double current_weight,
                      double current_value, int *chosen, int chosen_count) {
    if (current_value > ctx->best_value) {
        int i;
        ctx->best_value = current_value;
        ctx->best_count = chosen_count;
        for (i = 0; i < chosen_count; i++) {
            ctx->best_selected[i] = chosen[i];
        }
    }

    if (idx >= ctx->n) {
        return;
    }

    {
        double upper = fractional_bound(idx, current_weight, current_value,
                                      ctx->capacity, ctx->data, ctx->order);
        if (upper <= ctx->best_value) {
            return;
        }
    }

    {
        int item = ctx->order[idx];
        double wi = ctx->data->weights[item - 1];
        double vi = ctx->data->values[item - 1];

        if (current_weight + wi <= ctx->capacity) {
            chosen[chosen_count] = item;
            backtrack(ctx, idx + 1, current_weight + wi, current_value + vi,
                      chosen, chosen_count + 1);
        }
        backtrack(ctx, idx + 1, current_weight, current_value, chosen, chosen_count);
    }
}

KnapsackResult backtracking_solve(const KnapsackData *data, int capacity) {
    KnapsackResult result;
    OrderItem *order_items = NULL;
    int *order = NULL;
    int *chosen = NULL;
    int *best_selected = NULL;
    BtContext ctx;
    int i;
    double t0, t1;

    knapsack_result_init(&result);

    order_items = (OrderItem *)malloc((size_t)data->n * sizeof(OrderItem));
    order = (int *)malloc((size_t)data->n * sizeof(int));
    chosen = (int *)malloc((size_t)data->n * sizeof(int));
    best_selected = (int *)malloc((size_t)data->n * sizeof(int));

    if (!order_items || !order || !chosen || !best_selected) {
        free(order_items);
        free(order);
        free(chosen);
        free(best_selected);
        knapsack_result_set_skipped(&result, "内存分配失败");
        return result;
    }

    for (i = 0; i < data->n; i++) {
        order_items[i].item = i + 1;
        order_items[i].ratio = data->values[i] / data->weights[i];
    }
    qsort(order_items, (size_t)data->n, sizeof(OrderItem), compare_order_desc);
    for (i = 0; i < data->n; i++) {
        order[i] = order_items[i].item;
    }

    result.space_bytes = (size_t)data->n * (sizeof(OrderItem) + sizeof(int) * 3);

    ctx.data = data;
    ctx.capacity = capacity;
    ctx.order = order;
    ctx.best_value = 0.0;
    ctx.best_selected = best_selected;
    ctx.best_count = 0;
    ctx.n = data->n;

    t0 = get_time_ms();
    backtrack(&ctx, 0, 0.0, 0.0, chosen, 0);
    t1 = get_time_ms();

    result.num_selected = ctx.best_count;
    result.selected_items = (int *)malloc((size_t)ctx.best_count * sizeof(int));
    if (!result.selected_items) {
        free(order_items);
        free(order);
        free(chosen);
        free(best_selected);
        knapsack_result_set_skipped(&result, "内存分配失败");
        return result;
    }

    for (i = 0; i < ctx.best_count; i++) {
        result.selected_items[i] = best_selected[i];
    }
    qsort(result.selected_items, (size_t)ctx.best_count, sizeof(int), compare_int_asc);

    compute_totals(data, result.selected_items, result.num_selected,
                   &result.total_weight, &result.total_value);
    result.total_value = round2(result.total_value);
    result.elapsed_ms = t1 - t0;
    result.optimal = 1;

    free(order_items);
    free(order);
    free(chosen);
    free(best_selected);
    return result;
}
