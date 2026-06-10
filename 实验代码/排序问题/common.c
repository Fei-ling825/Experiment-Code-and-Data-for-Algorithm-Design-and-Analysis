#include "common.h"

#include <stdlib.h>
#include <string.h>

void int_list_init(IntList *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

void int_list_append(IntList *list, int value) {
    if (list->count >= list->capacity) {
        int new_cap = list->capacity == 0 ? 64 : list->capacity * 2;
        int *new_items = (int *)realloc(list->items, (size_t)new_cap * sizeof(int));
        if (!new_items) {
            return;
        }
        list->items = new_items;
        list->capacity = new_cap;
    }
    list->items[list->count++] = value;
}

void int_list_free(IntList *list) {
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

void array_copy(int *dst, const int *src, int n) {
    if (n > 0) {
        memcpy(dst, src, (size_t)n * sizeof(int));
    }
}

int compare_int_asc(const void *a, const void *b) {
    return (*(const int *)a) - (*(const int *)b);
}

int verify_sorted(const int *original, const int *sorted, int n) {
    int *expected;
    int i;
    int ok = 1;

    if (n <= 0) {
        return 1;
    }

    expected = (int *)malloc((size_t)n * sizeof(int));
    if (!expected) {
        return 0;
    }

    array_copy(expected, original, n);
    qsort(expected, (size_t)n, sizeof(int), compare_int_asc);

    for (i = 0; i < n; i++) {
        if (sorted[i] != expected[i]) {
            ok = 0;
            break;
        }
    }

    free(expected);
    return ok;
}
