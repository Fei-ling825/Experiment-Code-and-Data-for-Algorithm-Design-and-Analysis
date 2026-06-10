#include "bubble_sort.h"

long long bubble_sort(int *arr, int n) {
    long long comparison_count = 0;
    int i, j;

    for (i = 0; i < n; i++) {
        int swapped = 0;
        for (j = 0; j < n - 1 - i; j++) {
            comparison_count++;
            if (arr[j] > arr[j + 1]) {
                int tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
                swapped = 1;
            }
        }
        if (!swapped) {
            break;
        }
    }
    return comparison_count;
}
