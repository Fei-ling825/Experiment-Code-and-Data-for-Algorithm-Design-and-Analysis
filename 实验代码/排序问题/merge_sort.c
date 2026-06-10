#include "merge_sort.h"

#include <stdlib.h>

static void merge(int *arr, int left, int mid, int right, long long *comparison_count) {
    int len_left = mid - left + 1;
    int len_right = right - mid;
    int *left_part = (int *)malloc((size_t)len_left * sizeof(int));
    int *right_part = (int *)malloc((size_t)len_right * sizeof(int));
    int i = 0, j = 0, k = left;

    if (!left_part || !right_part) {
        free(left_part);
        free(right_part);
        return;
    }

    for (i = 0; i < len_left; i++) {
        left_part[i] = arr[left + i];
    }
    for (j = 0; j < len_right; j++) {
        right_part[j] = arr[mid + 1 + j];
    }

    i = 0;
    j = 0;
    while (i < len_left && j < len_right) {
        (*comparison_count)++;
        if (left_part[i] <= right_part[j]) {
            arr[k++] = left_part[i++];
        } else {
            arr[k++] = right_part[j++];
        }
    }
    while (i < len_left) {
        arr[k++] = left_part[i++];
    }
    while (j < len_right) {
        arr[k++] = right_part[j++];
    }

    free(left_part);
    free(right_part);
}

static void merge_sort_recursive(int *arr, int left, int right,
                                 long long *comparison_count, IntList *sizes) {
    int mid;
    int problem_size = right - left + 1;

    int_list_append(sizes, problem_size);
    if (left >= right) {
        return;
    }

    mid = left + (right - left) / 2;
    merge_sort_recursive(arr, left, mid, comparison_count, sizes);
    merge_sort_recursive(arr, mid + 1, right, comparison_count, sizes);
    merge(arr, left, mid, right, comparison_count);
}

void merge_sort_result_init(MergeSortResult *result) {
    result->comparison_count = 0;
    int_list_init(&result->subproblem_sizes);
}

void merge_sort_result_free(MergeSortResult *result) {
    int_list_free(&result->subproblem_sizes);
    result->comparison_count = 0;
}

void merge_sort_run(int *arr, int n, MergeSortResult *result) {
    int_list_free(&result->subproblem_sizes);
    result->comparison_count = 0;

    if (n <= 0) {
        return;
    }
    if (n == 1) {
        int_list_append(&result->subproblem_sizes, 1);
        return;
    }
    merge_sort_recursive(arr, 0, n - 1, &result->comparison_count, &result->subproblem_sizes);
}
