#ifndef SORT_COMMON_H
#define SORT_COMMON_H

typedef struct {
    int *items;
    int count;
    int capacity;
} IntList;

void int_list_init(IntList *list);
void int_list_append(IntList *list, int value);
void int_list_free(IntList *list);

void array_copy(int *dst, const int *src, int n);
int verify_sorted(const int *original, const int *sorted, int n);
int compare_int_asc(const void *a, const void *b);

#endif
