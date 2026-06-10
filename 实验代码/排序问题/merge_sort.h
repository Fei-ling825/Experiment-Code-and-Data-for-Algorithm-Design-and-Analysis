#ifndef MERGE_SORT_H
#define MERGE_SORT_H

#include "common.h"

typedef struct {
    long long comparison_count;
    IntList subproblem_sizes;
} MergeSortResult;

void merge_sort_result_init(MergeSortResult *result);
void merge_sort_result_free(MergeSortResult *result);
void merge_sort_run(int *arr, int n, MergeSortResult *result);

#endif
