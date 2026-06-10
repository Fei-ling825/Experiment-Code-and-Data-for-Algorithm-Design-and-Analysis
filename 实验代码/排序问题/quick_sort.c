#include "quick_sort.h"

static int partition(int *arr, int low, int high, long long *comparison_count) {
    int pivot = arr[high];
    int i = low - 1;
    int j;

    for (j = low; j < high; j++) {
        (*comparison_count)++;
        if (arr[j] <= pivot) {
            i++;
            {
                int tmp = arr[i];
                arr[i] = arr[j];
                arr[j] = tmp;
            }
        }
    }

    {
        int tmp = arr[i + 1];
        arr[i + 1] = arr[high];
        arr[high] = tmp;
    }
    return i + 1;
}

static void quick_sort_recursive(int *arr, int low, int high,
                                 long long *comparison_count, IntList *sizes) {
    int pivot_index;
    int problem_size;

    if (low >= high) {
        if (low == high) {
            int_list_append(sizes, 1);
        }
        return;
    }

    problem_size = high - low + 1;
    int_list_append(sizes, problem_size);

    pivot_index = partition(arr, low, high, comparison_count);
    quick_sort_recursive(arr, low, pivot_index - 1, comparison_count, sizes);
    quick_sort_recursive(arr, pivot_index + 1, high, comparison_count, sizes);
}

void quick_sort_result_init(QuickSortResult *result) {
    result->comparison_count = 0;
    int_list_init(&result->subproblem_sizes);
}

void quick_sort_result_free(QuickSortResult *result) {
    int_list_free(&result->subproblem_sizes);
    result->comparison_count = 0;
}

void quick_sort_run(int *arr, int n, QuickSortResult *result) {
    int_list_free(&result->subproblem_sizes);
    result->comparison_count = 0;

    if (n <= 0) {
        return;
    }
    if (n == 1) {
        int_list_append(&result->subproblem_sizes, 1);
        return;
    }
    quick_sort_recursive(arr, 0, n - 1, &result->comparison_count, &result->subproblem_sizes);
}
