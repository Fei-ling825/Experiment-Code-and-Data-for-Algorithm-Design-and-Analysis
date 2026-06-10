#include "common.h"
#include "bubble_sort.h"
#include "data_generator.h"
#include "merge_sort.h"
#include "quick_sort.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

static const int SIZES[] = {10, 100, 1000, 2000, 5000, 10000, 100000};
static const int SIZE_COUNT = (int)(sizeof(SIZES) / sizeof(SIZES[0]));
static const int BUBBLE_MAX_SIZE = 100000;
static const char *OUTPUT_DIR = "output";

static void ensure_output_dir(void) {
    MKDIR(OUTPUT_DIR);
}

static FILE *open_csv(const char *filename) {
    char path[512];
    FILE *fp;

    snprintf(path, sizeof(path), "%s/%s", OUTPUT_DIR, filename);
    fp = fopen(path, "wb");
    if (!fp) {
        fprintf(stderr, "[错误] 无法创建文件: %s\n", path);
        return NULL;
    }
    /* UTF-8 BOM，便于 Excel 打开 */
    fwrite("\xEF\xBB\xBF", 1, 3, fp);
    return fp;
}

static void run_experiment_1(void) {
    FILE *fp;
    int run;

    printf("\n============================================================\n");
    printf("实验①：等价类对比 — 两次 100 个随机数\n");
    printf("============================================================\n");

    fp = open_csv("experiment_1_results.csv");
    if (!fp) {
        return;
    }
    fprintf(fp, "运行次数,随机种子,冒泡比较次数,合并比较次数,快速比较次数\n");

    for (run = 1; run <= 2; run++) {
        unsigned int seed = 42u + (unsigned int)run;
        int *data = generate_random_data(100, seed);
        int *work;
        long long b_count, m_count, q_count;
        MergeSortResult m_result;
        QuickSortResult q_result;
        merge_sort_result_init(&m_result);
        quick_sort_result_init(&q_result);

        if (!data) {
            fprintf(stderr, "[错误] 内存分配失败\n");
            fclose(fp);
            return;
        }

        printf("\n--- 第 %d 次生成（seed=%u）---\n", run, seed);
        printf("测试数据: ");
        print_data_preview(data, 100);

        work = (int *)malloc(100 * sizeof(int));
        if (!work) {
            free(data);
            fclose(fp);
            return;
        }

        array_copy(work, data, 100);
        b_count = bubble_sort(work, 100);
        if (!verify_sorted(data, work, 100)) {
            printf("  [错误] 冒泡排序结果不正确！\n");
        }

        array_copy(work, data, 100);
        merge_sort_run(work, 100, &m_result);
        m_count = m_result.comparison_count;
        if (!verify_sorted(data, work, 100)) {
            printf("  [错误] 合并排序结果不正确！\n");
        }

        array_copy(work, data, 100);
        quick_sort_run(work, 100, &q_result);
        q_count = q_result.comparison_count;
        if (!verify_sorted(data, work, 100)) {
            printf("  [错误] 快速排序结果不正确！\n");
        }

        printf("冒泡排序比较次数: %lld\n", b_count);
        printf("合并排序比较次数: %lld\n", m_count);
        printf("快速排序比较次数: %lld\n", q_count);

        fprintf(fp, "%d,%u,%lld,%lld,%lld\n", run, seed, b_count, m_count, q_count);

        merge_sort_result_free(&m_result);
        quick_sort_result_free(&q_result);
        free(work);
        free(data);
    }

    fclose(fp);
    printf("\n结果已保存: %s/experiment_1_results.csv\n", OUTPUT_DIR);
}

static void run_experiment_2(void) {
    FILE *fp;
    int i;
    unsigned int seed = 2026u;

    printf("\n============================================================\n");
    printf("实验②：不同规模比较次数与增长率曲线\n");
    printf("============================================================\n");

    fp = open_csv("experiment_2_comparison_counts.csv");
    if (!fp) {
        return;
    }
    fprintf(fp, "数据规模 n,冒泡排序,合并排序,快速排序\n");

    for (i = 0; i < SIZE_COUNT; i++) {
        int size = SIZES[i];
        int *data = generate_random_data(size, seed + (unsigned int)size);
        int *work;
        long long b_count = -1;
        long long m_count = 0;
        long long q_count = 0;
        MergeSortResult m_result;
        QuickSortResult q_result;
        clock_t t0 = clock();
        merge_sort_result_init(&m_result);
        quick_sort_result_init(&q_result);

        if (!data) {
            fprintf(stderr, "[错误] n=%d 内存分配失败\n", size);
            continue;
        }

        work = (int *)malloc((size_t)size * sizeof(int));
        if (!work) {
            free(data);
            continue;
        }

        printf("\n规模 n = %d ...", size);

        if (size <= BUBBLE_MAX_SIZE) {
            array_copy(work, data, size);
            b_count = bubble_sort(work, size);
            if (!verify_sorted(data, work, size)) {
                printf(" [冒泡排序错误]");
            }
        } else {
            printf(" [冒泡排序跳过 n>%d]", BUBBLE_MAX_SIZE);
        }

        array_copy(work, data, size);
        merge_sort_run(work, size, &m_result);
        m_count = m_result.comparison_count;
        if (!verify_sorted(data, work, size)) {
            printf(" [合并排序错误]");
        }

        array_copy(work, data, size);
        quick_sort_run(work, size, &q_result);
        q_count = q_result.comparison_count;
        if (!verify_sorted(data, work, size)) {
            printf(" [快速排序错误]");
        }

        {
            double elapsed = (double)(clock() - t0) / (double)CLOCKS_PER_SEC;
            if (b_count >= 0) {
                printf(" 完成 (%.2fs) | 冒泡=%lld, 合并=%lld, 快速=%lld\n",
                       elapsed, b_count, m_count, q_count);
                fprintf(fp, "%d,%lld,%lld,%lld\n", size, b_count, m_count, q_count);
            } else {
                printf(" 完成 (%.2fs) | 冒泡=跳过, 合并=%lld, 快速=%lld\n",
                       elapsed, m_count, q_count);
                fprintf(fp, "%d,,%lld,%lld\n", size, m_count, q_count);
            }
        }

        merge_sort_result_free(&m_result);
        quick_sort_result_free(&q_result);
        free(work);
        free(data);
    }

    fclose(fp);
    printf("\n比较次数已保存: %s/experiment_2_comparison_counts.csv\n", OUTPUT_DIR);
}

static int int_list_max(const IntList *list) {
    int i;
    int max_val = 0;
    for (i = 0; i < list->count; i++) {
        if (list->items[i] > max_val) {
            max_val = list->items[i];
        }
    }
    return max_val;
}

static int int_list_min(const IntList *list) {
    int i;
    int min_val = 0;
    if (list->count == 0) {
        return 0;
    }
    min_val = list->items[0];
    for (i = 1; i < list->count; i++) {
        if (list->items[i] < min_val) {
            min_val = list->items[i];
        }
    }
    return min_val;
}

static void run_experiment_3(void) {
    FILE *summary_fp;
    FILE *detail_fp;
    int i;
    unsigned int seed = 2026u;

    printf("\n============================================================\n");
    printf("实验③：递归子问题规模分析（合并排序 & 快速排序）\n");
    printf("============================================================\n");

    summary_fp = open_csv("experiment_3_summary.csv");
    detail_fp = open_csv("experiment_3_subproblem_sizes.csv");
    if (!summary_fp || !detail_fp) {
        if (summary_fp) {
            fclose(summary_fp);
        }
        if (detail_fp) {
            fclose(detail_fp);
        }
        return;
    }

    fprintf(summary_fp,
            "数据规模 n,合并-递归次数,合并-比较次数,合并-最大子问题,合并-最小子问题,"
            "快速-递归次数,快速-比较次数,快速-最大子问题,快速-最小子问题\n");
    fprintf(detail_fp, "数据规模 n,算法,递归序号,子问题规模\n");

    for (i = 0; i < SIZE_COUNT; i++) {
        int size = SIZES[i];
        int *data = generate_random_data(size, seed + (unsigned int)size);
        int *work;
        MergeSortResult m_result;
        QuickSortResult q_result;
        int j;
        merge_sort_result_init(&m_result);
        quick_sort_result_init(&q_result);

        if (!data) {
            continue;
        }

        work = (int *)malloc((size_t)size * sizeof(int));
        if (!work) {
            free(data);
            continue;
        }

        array_copy(work, data, size);
        merge_sort_run(work, size, &m_result);
        if (!verify_sorted(data, work, size)) {
            printf("  [错误] 合并排序 n=%d 结果不正确\n", size);
        }

        array_copy(work, data, size);
        quick_sort_run(work, size, &q_result);
        if (!verify_sorted(data, work, size)) {
            printf("  [错误] 快速排序 n=%d 结果不正确\n", size);
        }

        printf("\nn = %d:\n", size);
        printf("  合并排序: 递归调用 %d 次, 比较 %lld 次\n",
               m_result.subproblem_sizes.count, m_result.comparison_count);
        printf("    子问题规模: ");
        {
            int show = m_result.subproblem_sizes.count < 20 ? m_result.subproblem_sizes.count : 20;
            for (j = 0; j < show; j++) {
                printf("%d%s", m_result.subproblem_sizes.items[j], j + 1 < show ? ", " : "");
            }
            if (m_result.subproblem_sizes.count > 20) {
                printf("...");
            }
            printf("\n");
        }

        printf("  快速排序: 递归调用 %d 次, 比较 %lld 次\n",
               q_result.subproblem_sizes.count, q_result.comparison_count);
        printf("    子问题规模: ");
        {
            int show = q_result.subproblem_sizes.count < 20 ? q_result.subproblem_sizes.count : 20;
            for (j = 0; j < show; j++) {
                printf("%d%s", q_result.subproblem_sizes.items[j], j + 1 < show ? ", " : "");
            }
            if (q_result.subproblem_sizes.count > 20) {
                printf("...");
            }
            printf("\n");
        }

        fprintf(summary_fp, "%d,%d,%lld,%d,%d,%d,%lld,%d,%d\n",
                size,
                m_result.subproblem_sizes.count,
                m_result.comparison_count,
                int_list_max(&m_result.subproblem_sizes),
                int_list_min(&m_result.subproblem_sizes),
                q_result.subproblem_sizes.count,
                q_result.comparison_count,
                int_list_max(&q_result.subproblem_sizes),
                int_list_min(&q_result.subproblem_sizes));

        for (j = 0; j < m_result.subproblem_sizes.count; j++) {
            fprintf(detail_fp, "%d,合并排序,%d,%d\n",
                    size, j + 1, m_result.subproblem_sizes.items[j]);
        }
        for (j = 0; j < q_result.subproblem_sizes.count; j++) {
            fprintf(detail_fp, "%d,快速排序,%d,%d\n",
                    size, j + 1, q_result.subproblem_sizes.items[j]);
        }

        merge_sort_result_free(&m_result);
        quick_sort_result_free(&q_result);
        free(work);
        free(data);
    }

    fclose(summary_fp);
    fclose(detail_fp);
    printf("\n汇总表已保存: %s/experiment_3_summary.csv\n", OUTPUT_DIR);
    printf("详细子问题规模已保存: %s/experiment_3_subproblem_sizes.csv\n", OUTPUT_DIR);

    printf("\n--- 实验③ 分析结论 ---\n");
    printf("1. 合并排序：每次递归将问题均分为两个规模约为 n/2 的子问题，\n");
    printf("   子问题规模呈规律递减（n, n/2, n/4, ...），递归树平衡。\n");
    printf("2. 快速排序：子问题规模取决于基准元素划分结果，随输入数据变化。\n");
    printf("3. 合并排序的子问题规模可预测且稳定；快速排序体现分治中规模的动态变化。\n");
}

int main(void) {
    int i;

    ensure_output_dir();

    printf("排序算法对比实验 (C 语言)\n");
    printf("测试规模: ");
    for (i = 0; i < SIZE_COUNT; i++) {
        printf("%d%s", SIZES[i], i + 1 < SIZE_COUNT ? ", " : "");
    }
    printf("\n输出目录: %s\n", OUTPUT_DIR);

    run_experiment_1();
    run_experiment_2();
    run_experiment_3();

    printf("\n============================================================\n");
    printf("全部实验完成！请运行 python plot_results.py 生成图表。\n");
    printf("============================================================\n");

    return 0;
}
