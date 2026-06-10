#ifndef QUICK_SORT_H
#define QUICK_SORT_H

#include "common.h"

typedef struct {
    long long comparison_count;
    IntList subproblem_sizes;
} QuickSortResult;

void quick_sort_result_init(QuickSortResult *result);
void quick_sort_result_free(QuickSortResult *result);
void quick_sort_run(int *arr, int n, QuickSortResult *result);

#endif
