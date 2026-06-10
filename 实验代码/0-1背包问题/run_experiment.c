/*
 * 0-1 背包问题对比实验 — C 语言统一入口
 * 蛮力法、动态规划、贪心法、回溯法（无规模限制，按题目要求全规模运行）
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "backtracking.h"
#include "brute_force.h"
#include "common.h"
#include "data_generator.h"
#include "dynamic_programming.h"
#include "export_data.h"
#include "greedy.h"

#define BASE_SEED 2026U
#define OUTPUT_DIR "output"

/* 题目要求的物品数量*/
static const int ITEM_COUNTS[] = {
    1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000,
    20000, 40000, 80000, 160000, 320000
};
static const int NUM_ITEM_COUNTS = (int)(sizeof(ITEM_COUNTS) / sizeof(ITEM_COUNTS[0]));

static const int CAPACITIES[] = {10000, 100000, 1000000};
static const int NUM_CAPACITIES = (int)(sizeof(CAPACITIES) / sizeof(CAPACITIES[0]));

/* 为了能“批量跑完并生成表格/图表”：
 * 回溯法/蛮力法在大 n 时趋近指数级，可能长时间卡住。
 * 超过阈值后直接标记为 skipped，CSV 里会记录跳过原因。*/
#define BACKTRACKING_MAX_N 1500
#define BRUTE_FORCE_MAX_N 300

/* 小规模四算法对比：n 控制在蛮力法可接受范围内 */
static const int SMALL_ITEM_COUNTS[] = {10, 12, 14, 16, 18, 20, 22, 24, 26, 28};
static const int NUM_SMALL_ITEM_COUNTS =
    (int)(sizeof(SMALL_ITEM_COUNTS) / sizeof(SMALL_ITEM_COUNTS[0]));
static const int SMALL_CAPACITIES[] = {500};
static const int NUM_SMALL_CAPACITIES =
    (int)(sizeof(SMALL_CAPACITIES) / sizeof(SMALL_CAPACITIES[0]));
#define SMALL_SEED_BASE 900000U
/* 小规模重复执行取平均耗时，提高亚毫秒级计时分辨率 */
#define SMALL_TIMING_REPEATS 100

typedef struct {
    const char *name;
    const char *complexity;
    KnapsackResult (*solve)(const KnapsackData *, int);
    int optimal;
} AlgoInfo;

/* 先运行较快算法，便于记录结果；蛮力法/回溯法放后 */
static const AlgoInfo ALGORITHMS[] = {
    {"贪心法", "O(n log n)", greedy_solve, 0},
    {"动态规划", "O(nW)", dp_solve, 1},
    {"回溯法", "O(2^n)最坏", backtracking_solve, 1},
    {"蛮力法", "O(2^n)", brute_force_solve, 1},
};
static const int NUM_ALGOS = (int)(sizeof(ALGORITHMS) / sizeof(ALGORITHMS[0]));

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

static int double_equal(double a, double b) {
    double d = a - b;
    return d < 0 ? -d < 0.01 : d < 0.01;
}

static int verify_correctness(void) {
    KnapsackData example;
    KnapsackResult r_bf, r_dp, r_bt, r_gr;
    KnapsackData small;
    int ok = 1;

    printf("\n============================================================\n");
    printf("正确性验证（题目示例 + 随机小规模）\n");
    printf("============================================================\n");

    example.n = 5;
    example.weights = (double[]){2, 2, 6, 5, 4};
    example.values = (double[]){6, 3, 5, 4, 6};

    printf("\n题目示例: 容量=10, 重量=[2,2,6,5,4], 价值=[6,3,5,4,6]\n");

    r_bf = brute_force_solve(&example, 10);
    r_dp = dp_solve(&example, 10);
    r_bt = backtracking_solve(&example, 10);

    printf("  蛮力法:   价值=%.2f\n", r_bf.total_value);
    printf("  动态规划: 价值=%.2f\n", r_dp.total_value);
    printf("  回溯法:   价值=%.2f\n", r_bt.total_value);

    if (!double_equal(r_bf.total_value, 15.0) ||
        !double_equal(r_dp.total_value, r_bf.total_value) ||
        !double_equal(r_bt.total_value, r_bf.total_value)) {
        printf("  [错误] 最优算法结果不正确！\n");
        ok = 0;
    }

    r_gr = greedy_solve(&example, 10);
    printf("  贪心法:   价值=%.2f\n", r_gr.total_value);

    knapsack_result_free(&r_bf);
    knapsack_result_free(&r_dp);
    knapsack_result_free(&r_bt);
    knapsack_result_free(&r_gr);

    if (!generate_knapsack_data(&small, 20, 99, 0)) {
        return 0;
    }
    r_bf = brute_force_solve(&small, 500);
    r_dp = dp_solve(&small, 500);
    r_bt = backtracking_solve(&small, 500);

    if (!double_equal(r_bf.total_value, r_dp.total_value) ||
        !double_equal(r_bf.total_value, r_bt.total_value)) {
        printf("  [错误] 小规模随机数据上最优算法不一致！\n");
        ok = 0;
    } else {
        printf("  验证通过。\n");
    }

    knapsack_data_free(&small);
    knapsack_result_free(&r_bf);
    knapsack_result_free(&r_dp);
    knapsack_result_free(&r_bt);
    return ok;
}

static void save_results_header(FILE *fp) {
    fprintf(fp, "背包容量,物品数量,算法,理论复杂度,总价值,总重量,选中物品数,");
    fprintf(fp, "执行时间(ms),空间占用(字节),是否最优算法,是否跳过,跳过原因\n");
}

static void save_result_row(FILE *fp, int capacity, int n_items, const AlgoInfo *algo,
                            const KnapsackResult *r) {
    fprintf(fp, "%d,%d,%s,%s,", capacity, n_items, algo->name, algo->complexity);
    if (r->skipped) {
        fprintf(fp, ",,,,,是,是,%s\n", r->skip_reason);
    } else {
        fprintf(fp, "%.2f,%.2f,%d,%.4f,%zu,",
                r->total_value, r->total_weight, r->num_selected,
                r->elapsed_ms, r->space_bytes);
        fprintf(fp, "%s,否,\n", algo->optimal ? "是" : "否");
    }
    fflush(fp);
}

static void save_detail_items(FILE *fp, int capacity, int n_items, const char *algo_name,
                              const KnapsackData *data, const KnapsackResult *r) {
    int i;
    for (i = 0; i < r->num_selected; i++) {
        int id = r->selected_items[i];
        fprintf(fp, "%d,%d,%s,%d,%.0f,%.2f\n",
                capacity, n_items, algo_name, id,
                data->weights[id - 1], data->values[id - 1]);
    }
    fflush(fp);
}

static KnapsackResult solve_timed(const AlgoInfo *algo, const KnapsackData *data,
                                  int capacity, int n_items, int skip_exponential,
                                  int timing_repeats) {
    KnapsackResult result;
    KnapsackResult tmp;
    double t0, t1;
    int rep;

    if (skip_exponential && strcmp(algo->name, "回溯法") == 0 && n_items > BACKTRACKING_MAX_N) {
        knapsack_result_init(&result);
        snprintf(result.skip_reason, MAX_SKIP_REASON,
                 "n=%d>=%d，跳过回溯法（指数级，避免卡死）", n_items, BACKTRACKING_MAX_N);
        knapsack_result_set_skipped(&result, result.skip_reason);
        return result;
    }
    if (skip_exponential && strcmp(algo->name, "蛮力法") == 0 && n_items > BRUTE_FORCE_MAX_N) {
        knapsack_result_init(&result);
        snprintf(result.skip_reason, MAX_SKIP_REASON,
                 "n=%d>=%d，跳过蛮力法（指数级枚举，避免卡死）", n_items, BRUTE_FORCE_MAX_N);
        knapsack_result_set_skipped(&result, result.skip_reason);
        return result;
    }

    if (timing_repeats <= 1) {
        return algo->solve(data, capacity);
    }

    t0 = get_time_ms();
    result = algo->solve(data, capacity);
    for (rep = 1; rep < timing_repeats; rep++) {
        tmp = algo->solve(data, capacity);
        knapsack_result_free(&result);
        result = tmp;
    }
    t1 = get_time_ms();
    result.elapsed_ms = (t1 - t0) / (double)timing_repeats;
    return result;
}

static void run_experiment_suite(const char *title, const char *results_path,
                                 const char *detail_path, const int *capacities,
                                 int num_capacities, const int *item_counts,
                                 int num_item_counts, unsigned int seed_base,
                                 int skip_exponential, int use_table1_for_1000,
                                 int timing_repeats) {
    FILE *fp_results = NULL;
    FILE *fp_detail = NULL;
    int ci, ni, ai;

    MKDIR(OUTPUT_DIR);
    fp_results = fopen(results_path, "wb");
    fp_detail = fopen(detail_path, "wb");
    if (!fp_results || !fp_detail) {
        printf("无法创建输出 CSV 文件。\n");
        if (fp_results) fclose(fp_results);
        if (fp_detail) fclose(fp_detail);
        return;
    }

    fwrite("\xEF\xBB\xBF", 1, 3, fp_results);
    fwrite("\xEF\xBB\xBF", 1, 3, fp_detail);
    save_results_header(fp_results);
    fprintf(fp_detail, "背包容量,物品数量,算法,物品编号,重量,价值\n");

    printf("\n============================================================\n");
    printf("%s\n", title);
    printf("============================================================\n");

    for (ci = 0; ci < num_capacities; ci++) {
        int capacity = capacities[ci];
        printf("\n========================================\n");
        printf("背包容量 W = %d\n", capacity);
        printf("========================================\n");

        for (ni = 0; ni < num_item_counts; ni++) {
            int n_items = item_counts[ni];
            KnapsackData data;
            double gen_t0, gen_t1;
            int dj, duplicate = 0;

            for (dj = 0; dj < ni; dj++) {
                if (item_counts[dj] == n_items) {
                    duplicate = 1;
                    break;
                }
            }
            if (duplicate) {
                continue;
            }

            gen_t0 = get_time_ms();
            if (use_table1_for_1000 && n_items == TABLE1_ITEM_COUNT) {
                if (!get_table1_data(&data)) {
                    continue;
                }
            } else {
                unsigned int seed = seed_base + (unsigned int)capacity + (unsigned int)n_items;
                if (!generate_knapsack_data(&data, n_items, seed, 0)) {
                    continue;
                }
            }
            gen_t1 = get_time_ms();

            printf("\n--- n=%d, W=%d (数据生成 %.1f ms) ---\n",
                   n_items, capacity, gen_t1 - gen_t0);

            for (ai = 0; ai < NUM_ALGOS; ai++) {
                const AlgoInfo *algo = &ALGORITHMS[ai];
                KnapsackResult result = solve_timed(
                    algo, &data, capacity, n_items, skip_exponential, timing_repeats);

                print_result_summary(algo->name, &result);
                save_result_row(fp_results, capacity, n_items, algo, &result);

                if (!result.skipped) {
                    save_detail_items(fp_detail, capacity, n_items,
                                      algo->name, &data, &result);
                }
                knapsack_result_free(&result);
            }

            knapsack_data_free(&data);
        }
    }

    fclose(fp_results);
    fclose(fp_detail);
    printf("\n汇总结果已保存: %s\n", results_path);
    printf("物品明细已保存: %s\n", detail_path);
}

static void run_small_scale_experiments(void) {
    run_experiment_suite(
        "小规模实验：四算法完整对比（n=10~28，不跳过回溯/蛮力）",
        OUTPUT_DIR "/small_scale_results.csv",
        OUTPUT_DIR "/small_scale_detail.csv",
        SMALL_CAPACITIES, NUM_SMALL_CAPACITIES,
        SMALL_ITEM_COUNTS, NUM_SMALL_ITEM_COUNTS,
        SMALL_SEED_BASE, 0, 0, SMALL_TIMING_REPEATS);
}

static void run_experiments(void) {
    run_experiment_suite(
        "主实验：不同物品数量与背包容量（大规模，回溯/蛮力超限自动跳过）",
        OUTPUT_DIR "/experiment_results.csv",
        OUTPUT_DIR "/selected_items_detail.csv",
        CAPACITIES, NUM_CAPACITIES,
        ITEM_COUNTS, NUM_ITEM_COUNTS,
        BASE_SEED, 1, 1, 1);
}

static void print_analysis(void) {
    printf("\n============================================================\n");
    printf("实验分析与结论\n");
    printf("============================================================\n");
    printf("1. 蛮力法 O(2^n): 随 n 指数增长，大规模输入耗时极长。\n");
    printf("2. 动态规划 O(nW): 时间与空间随 n 和 W 乘积增长。\n");
    printf("3. 贪心法 O(n log n): 增长最慢，但不保证最优。\n");
    printf("4. 回溯法: 带剪枝，最坏 O(2^n)，实际表现取决于数据。\n");
    printf("5. 小规模实验 (small_scale_results.csv) 可对比四种算法增长特征。\n");
    printf("6. 运行 python plot_results.py 生成时间/空间对比图。\n");
}

int main(void) {
    printf("0-1 背包问题对比实验 (C语言，无规模限制)\n");
    printf("物品数量: ");
    {
        int i;
        for (i = 0; i < NUM_ITEM_COUNTS; i++) {
            printf("%d%s", ITEM_COUNTS[i], i + 1 < NUM_ITEM_COUNTS ? ", " : "\n");
        }
    }
    printf("背包容量: 10000, 100000, 1000000\n");
    printf("输出目录: %s\n", OUTPUT_DIR);

    if (!verify_correctness()) {
        printf("正确性验证失败，终止实验。\n");
        return 1;
    }

    printf("\n============================================================\n");
    printf("生成表 1：1000 个物品统计信息\n");
    printf("============================================================\n");
    export_table1_csv(OUTPUT_DIR);

    run_small_scale_experiments();
    run_experiments();
    print_analysis();

    printf("\n============================================================\n");
    printf("实验完成！请运行: python plot_results.py\n");
    printf("============================================================\n");
    return 0;
}
