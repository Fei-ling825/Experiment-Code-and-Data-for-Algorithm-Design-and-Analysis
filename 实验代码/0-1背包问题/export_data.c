#include "export_data.h"
#include "data_generator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

static FILE *open_table1_csv(const char *output_dir, char *filepath, size_t filepath_size) {
    FILE *fp;

    snprintf(filepath, filepath_size, "%s/%s", output_dir,
             "表1_0-1背包问题1000个物品统计信息.csv");
    fp = fopen(filepath, "wb");
    if (fp) {
        return fp;
    }

    snprintf(filepath, filepath_size, "%s/%s", output_dir, "table1_items_1000.csv");
    fp = fopen(filepath, "wb");
    if (fp) {
        printf("（中文文件名打开失败，已改用英文备用文件）\n");
        return fp;
    }

    return NULL;
}

int export_table1_csv(const char *output_dir) {
    KnapsackData data;
    char filepath[512];
    FILE *fp;
    int i;

    MKDIR(output_dir);

    if (!get_table1_data(&data)) {
        printf("[错误] 无法生成表 1 数据。\n");
        return 0;
    }

    fp = open_table1_csv(output_dir, filepath, sizeof(filepath));
    if (!fp) {
        knapsack_data_free(&data);
        printf("[错误] 无法创建表 1 CSV（已尝试中文与英文文件名）。\n");
        return 0;
    }

    /* UTF-8 BOM，Excel 正确识别中文 */
    fwrite("\xEF\xBB\xBF", 1, 3, fp);
    fprintf(fp, "表 1. 0-1 背包问题 %d 个物品的统计信息\n", TABLE1_ITEM_COUNT);
    fprintf(fp, "物品编号,物品重量,物品价值\n");

    for (i = 0; i < data.n; i++) {
        fprintf(fp, "%d,%.0f,%.2f\n", i + 1, data.weights[i], data.values[i]);
    }

    fclose(fp);
    knapsack_data_free(&data);
    printf("表1 CSV 已保存: %s\n", filepath);
    printf("（可用 Excel 直接打开；列：物品编号 | 物品重量 | 物品价值）\n");
    return 1;
}
