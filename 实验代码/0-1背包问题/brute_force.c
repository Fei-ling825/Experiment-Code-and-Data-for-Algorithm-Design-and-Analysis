#include "brute_force.h"
#include "data_generator.h"

#include <stdlib.h>

typedef struct {
    const KnapsackData *data;
    int capacity;
    double best_value;
    int *best_selected;
    int best_count;
    int n;
} BfContext;

static void brute_force_rec(BfContext *ctx, int idx, double current_weight,
                            double current_value, int *chosen, int chosen_count) {
    if (idx >= ctx->n) {
        if (current_value > ctx->best_value) {
            int i;
            ctx->best_value = current_value;
            ctx->best_count = chosen_count;
            for (i = 0; i < chosen_count; i++) {
                ctx->best_selected[i] = chosen[i];
            }
        }
        return;
    }

    brute_force_rec(ctx, idx + 1, current_weight, current_value, chosen, chosen_count);

    {
        double wi = ctx->data->weights[idx];
        double vi = ctx->data->values[idx];
        if (current_weight + wi <= ctx->capacity) {
            chosen[chosen_count] = idx + 1;
            brute_force_rec(ctx, idx + 1, current_weight + wi, current_value + vi,
                            chosen, chosen_count + 1);
        }
    }
}

KnapsackResult brute_force_solve(const KnapsackData *data, int capacity) {
    KnapsackResult result;
    BfContext ctx;
    int *chosen = NULL;
    int *best_selected = NULL;
    double t0, t1;

    knapsack_result_init(&result);

    chosen = (int *)malloc((size_t)data->n * sizeof(int));
    best_selected = (int *)malloc((size_t)data->n * sizeof(int));
    if (!chosen || !best_selected) {
        free(chosen);
        free(best_selected);
        knapsack_result_set_skipped(&result, "内存分配失败");
        return result;
    }

    ctx.data = data;
    ctx.capacity = capacity;
    ctx.best_value = 0.0;
    ctx.best_selected = best_selected;
    ctx.best_count = 0;
    ctx.n = data->n;

    result.space_bytes = (size_t)data->n * sizeof(int) * 2;
    t0 = get_time_ms();
    brute_force_rec(&ctx, 0, 0.0, 0.0, chosen, 0);
    t1 = get_time_ms();

    result.num_selected = ctx.best_count;
    result.selected_items = (int *)malloc((size_t)ctx.best_count * sizeof(int));
    if (!result.selected_items) {
        free(chosen);
        free(best_selected);
        knapsack_result_set_skipped(&result, "内存分配失败");
        return result;
    }

    {
        int i;
        for (i = 0; i < ctx.best_count; i++) {
            result.selected_items[i] = best_selected[i];
        }
    }

    compute_totals(data, result.selected_items, result.num_selected,
                   &result.total_weight, &result.total_value);
    result.total_value = round2(result.total_value);
    result.elapsed_ms = t1 - t0;
    result.optimal = 1;

    free(chosen);
    free(best_selected);
    return result;
}
