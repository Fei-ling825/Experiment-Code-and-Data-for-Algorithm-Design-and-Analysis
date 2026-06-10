#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

void knapsack_result_init(KnapsackResult *r) {
    memset(r, 0, sizeof(*r));
    r->optimal = 1;
}

void knapsack_result_free(KnapsackResult *r) {
    free(r->selected_items);
    r->selected_items = NULL;
    r->num_selected = 0;
}

void knapsack_result_set_skipped(KnapsackResult *r, const char *reason) {
    r->skipped = 1;
    r->optimal = 1;
    r->elapsed_ms = 0.0;
    r->space_bytes = 0;
    strncpy(r->skip_reason, reason, MAX_SKIP_REASON - 1);
    r->skip_reason[MAX_SKIP_REASON - 1] = '\0';
}

void knapsack_data_free(KnapsackData *data) {
    free(data->weights);
    free(data->values);
    data->weights = NULL;
    data->values = NULL;
    data->n = 0;
}

void compute_totals(const KnapsackData *data, const int *selected, int num_selected,
                    double *total_weight, double *total_value) {
    double tw = 0.0, tv = 0.0;
    int i;
    for (i = 0; i < num_selected; i++) {
        int idx = selected[i] - 1;
        tw += data->weights[idx];
        tv += data->values[idx];
    }
    *total_weight = tw;
    *total_value = tv;
}

double get_time_ms(void) {
#ifdef _WIN32
    static LARGE_INTEGER freq = {0};
    LARGE_INTEGER counter;
    if (freq.QuadPart == 0) {
        QueryPerformanceFrequency(&freq);
    }
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart * 1000.0 / (double)freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
#endif
}

int weight_for_dp(double weight) {
    int w = (int)(weight + 0.5);
    return w < 1 ? 1 : w;
}

static const char *format_space(size_t nbytes, char *buf, size_t buflen) {
    if (nbytes < 1024) {
        snprintf(buf, buflen, "%zu B", nbytes);
    } else if (nbytes < 1024 * 1024) {
        snprintf(buf, buflen, "%.1f KB", nbytes / 1024.0);
    } else if (nbytes < 1024ULL * 1024 * 1024) {
        snprintf(buf, buflen, "%.1f MB", nbytes / (1024.0 * 1024.0));
    } else {
        snprintf(buf, buflen, "%.2f GB", nbytes / (1024.0 * 1024.0 * 1024.0));
    }
    return buf;
}

void print_result_summary(const char *algo_name, const KnapsackResult *r) {
    char space_buf[64];
    if (r->skipped) {
        printf("  [%s] 跳过 — %s\n", algo_name, r->skip_reason);
        return;
    }
    printf("  [%s] 选中 %d 件 | 总重量: %.2f | 总价值: %.2f | 耗时: %.4f ms | 空间: %s\n",
           algo_name, r->num_selected, r->total_weight, r->total_value, r->elapsed_ms,
           format_space(r->space_bytes, space_buf, sizeof(space_buf)));
}
