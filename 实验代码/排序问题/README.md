# 排序算法对比实验（C 语言 + Python 绘图）

## 实验内容

1. **实验①**：两次生成 100 个随机数，记录三种算法的比较次数（等价类理解）
2. **实验②**：规模 `10, 100, 1000, 2000, 5000, 10000, 100000`，记录比较次数
3. **实验③**：合并排序与快速排序的递归子问题规模分析

## 目录结构

```
c/
├── common.c/h          公共工具（数组拷贝、排序验证、IntList）
├── data_generator.c/h  随机测试数据生成
├── bubble_sort.c/h     冒泡排序（统计比较次数）
├── merge_sort.c/h      合并排序（比较次数 + 子问题规模）
├── quick_sort.c/h      快速排序（Lomuto 分区，比较次数 + 子问题规模）
├── run_experiment.c    实验主程序，输出 CSV
├── plot_results.py     读取 CSV 生成图表
├── build.bat           Windows 一键编译运行 + 绘图
├── Makefile            Linux/macOS 编译
└── output/             实验结果（CSV + PNG）
```

## 运行方式

### Windows

```bat
cd D:\算法\排序问题\c
build.bat
```

### 手动步骤

```bat
gcc -Wall -Wextra -O2 -std=c99 -o run_experiment.exe common.c data_generator.c bubble_sort.c merge_sort.c quick_sort.c run_experiment.c
run_experiment.exe
python plot_results.py
```

## 输出文件

| 文件 | 说明 |
|------|------|
| `experiment_1_results.csv` | 实验① 两次 100 元随机数据比较次数 |
| `experiment_2_comparison_counts.csv` | 实验② 各规模比较次数 |
| `experiment_3_summary.csv` | 实验③ 子问题规模汇总 |
| `experiment_3_subproblem_sizes.csv` | 实验③ 完整子问题规模明细 |
| `comparison_counts_linear.png` | 比较次数线性坐标图 |
| `comparison_counts_log.png` | 比较次数对数坐标图 |
| `subproblem_analysis.png` | 子问题规模分析图 |

## 依赖

- **GCC**（MinGW-w64 / MSYS2）
- **Python 3** + `matplotlib` + `numpy`

```bat
pip install matplotlib numpy
```
