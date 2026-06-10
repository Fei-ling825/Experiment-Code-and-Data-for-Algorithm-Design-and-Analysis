"""
读取 C 语言实验输出的 CSV，生成比较次数曲线与子问题分析图
用法:
  python plot_results.py
"""

import csv
import os
import warnings

import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from matplotlib import ticker

matplotlib.rcParams.update({
    "font.sans-serif": ["Microsoft YaHei", "SimHei", "DejaVu Sans"],
    "axes.unicode_minus": False,
    "axes.formatter.use_mathtext": False,
})

warnings.filterwarnings("ignore", message=r"Glyph .* missing from font")
warnings.filterwarnings(
    "ignore",
    message=r"Font 'default' does not have a glyph for '\\u2212'",
)


class AsciiMinusLogFormatter(ticker.LogFormatterSciNotation):
    def __call__(self, x, pos=None):
        label = super().__call__(x, pos)
        return label.replace("\u2212", "-") if isinstance(label, str) else label


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
OUTPUT_DIR = os.path.join(SCRIPT_DIR, "output")


def load_experiment_2():
    path = os.path.join(OUTPUT_DIR, "experiment_2_comparison_counts.csv")
    sizes = []
    bubble = {}
    merge = {}
    quick = {}

    with open(path, encoding="utf-8-sig") as f:
        reader = csv.DictReader(f)
        for row in reader:
            n = int(row["数据规模 n"])
            sizes.append(n)
            b_val = row["冒泡排序"].strip()
            bubble[n] = int(b_val) if b_val else None
            merge[n] = int(row["合并排序"])
            quick[n] = int(row["快速排序"])

    return sizes, bubble, merge, quick


def load_experiment_3_summary():
    path = os.path.join(OUTPUT_DIR, "experiment_3_summary.csv")
    records = []
    with open(path, encoding="utf-8-sig") as f:
        reader = csv.DictReader(f)
        for row in reader:
            records.append({
                "size": int(row["数据规模 n"]),
                "merge_count": int(row["合并-递归次数"]),
                "quick_count": int(row["快速-递归次数"]),
                "merge_max": int(row["合并-最大子问题"]),
                "quick_max": int(row["快速-最大子问题"]),
            })
    return records


def plot_comparison_linear(sizes, bubble, merge, quick):
    fig, ax = plt.subplots(figsize=(10, 6))

    bubble_sizes = [s for s in sizes if bubble.get(s) is not None]
    bubble_counts = [bubble[s] for s in bubble_sizes]
    merge_y = [merge[s] for s in sizes]
    quick_y = [quick[s] for s in sizes]

    if bubble_sizes:
        ax.plot(bubble_sizes, bubble_counts, "o-", label="冒泡排序 O(n^2)", linewidth=2, markersize=6)
    ax.plot(sizes, merge_y, "s-", label="合并排序 O(n log n)", linewidth=2, markersize=6)
    ax.plot(sizes, quick_y, "^-", label="快速排序 O(n log n)", linewidth=2, markersize=6)

    ax.set_xlabel("输入数据规模 n", fontsize=12)
    ax.set_ylabel("比较操作执行次数", fontsize=12)
    ax.set_title("三种排序算法比较次数随数据规模变化", fontsize=14)
    ax.legend(fontsize=11)
    ax.grid(True, alpha=0.3)

    path = os.path.join(OUTPUT_DIR, "comparison_counts_linear.png")
    fig.tight_layout()
    fig.savefig(path, dpi=150)
    plt.close(fig)
    print(f"线性坐标曲线图已保存: {path}")


def plot_comparison_log(sizes, bubble, merge, quick):
    fig, axes = plt.subplots(1, 2, figsize=(14, 5))
    sizes_arr = np.array(sizes)
    counts = {"bubble": bubble, "merge": merge, "quick": quick}

    ax1 = axes[0]
    for name, label, marker in [
        ("bubble", "冒泡排序", "o"),
        ("merge", "合并排序", "s"),
        ("quick", "快速排序", "^"),
    ]:
        valid = [(s, counts[name][s]) for s in sizes if counts[name].get(s) is not None]
        if valid:
            xs, ys = zip(*valid)
            ax1.semilogy(xs, ys, f"{marker}-", label=label, linewidth=2, markersize=6)
    ax1.set_xlabel("输入数据规模 n")
    ax1.set_ylabel("比较次数（对数刻度）")
    ax1.set_title("比较次数 — Y 轴对数")
    ax1.yaxis.set_major_formatter(AsciiMinusLogFormatter())
    ax1.legend()
    ax1.grid(True, alpha=0.3)

    ax2 = axes[1]
    for name, label, marker in [
        ("bubble", "冒泡排序", "o"),
        ("merge", "合并排序", "s"),
        ("quick", "快速排序", "^"),
    ]:
        valid = [(s, counts[name][s]) for s in sizes if counts[name].get(s) is not None]
        if valid:
            xs, ys = zip(*valid)
            ax2.loglog(xs, ys, f"{marker}-", label=label, linewidth=2, markersize=6)

    n_ref = np.logspace(1, 5, 50)
    ax2.loglog(n_ref, n_ref ** 2 / 10, "k--", alpha=0.5, label="参考: n^2/10")
    ax2.loglog(n_ref, n_ref * np.log2(n_ref), "k:", alpha=0.5, label="参考: n log n")

    ax2.set_xlabel("输入数据规模 n（对数刻度）")
    ax2.set_ylabel("比较次数（对数刻度）")
    ax2.set_title("比较次数 — 双对数坐标")
    ax2.xaxis.set_major_formatter(AsciiMinusLogFormatter())
    ax2.yaxis.set_major_formatter(AsciiMinusLogFormatter())
    ax2.legend(fontsize=9)
    ax2.grid(True, alpha=0.3)

    path = os.path.join(OUTPUT_DIR, "comparison_counts_log.png")
    fig.tight_layout()
    fig.savefig(path, dpi=150)
    plt.close(fig)
    print(f"对数坐标曲线图已保存: {path}")


def plot_subproblem_analysis(records):
    sizes = [r["size"] for r in records]
    merge_calls = [r["merge_count"] for r in records]
    quick_calls = [r["quick_count"] for r in records]
    merge_max = [r["merge_max"] for r in records]
    quick_max = [r["quick_max"] for r in records]

    fig, axes = plt.subplots(1, 2, figsize=(14, 5))

    ax1 = axes[0]
    ax1.plot(sizes, merge_calls, "s-", label="合并排序递归次数", linewidth=2)
    ax1.plot(sizes, quick_calls, "^-", label="快速排序递归次数", linewidth=2)
    ax1.set_xlabel("数据规模 n")
    ax1.set_ylabel("递归调用次数")
    ax1.set_title("递归调用次数随数据规模变化")
    ax1.legend()
    ax1.grid(True, alpha=0.3)

    ax2 = axes[1]
    ax2.plot(sizes, merge_max, "s-", label="合并排序最大子问题", linewidth=2)
    ax2.plot(sizes, sizes, "k--", alpha=0.4, label="y = n（参考）")
    ax2.plot(sizes, quick_max, "^-", label="快速排序最大子问题", linewidth=2)
    ax2.set_xlabel("数据规模 n")
    ax2.set_ylabel("最大子问题规模")
    ax2.set_title("各算法最大子问题规模")
    ax2.legend()
    ax2.grid(True, alpha=0.3)

    path = os.path.join(OUTPUT_DIR, "subproblem_analysis.png")
    fig.tight_layout()
    fig.savefig(path, dpi=150)
    plt.close(fig)
    print(f"子问题分析图已保存: {path}")


def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    sizes, bubble, merge, quick = load_experiment_2()
    plot_comparison_linear(sizes, bubble, merge, quick)
    plot_comparison_log(sizes, bubble, merge, quick)

    records = load_experiment_3_summary()
    plot_subproblem_analysis(records)

    print("\n全部图表已生成，请查看 output 目录。")


if __name__ == "__main__":
    main()
