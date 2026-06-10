"""
读取 C 语言或 Python 实验输出的 CSV，生成执行时间/空间对比图
用法:
  python plot_results.py                    # 绘制大规模 + 小规模（若存在）
  python plot_results.py <csv路径>          # 仅绘制指定 CSV
"""

import csv
import os
import sys
import warnings

import matplotlib
from matplotlib import ticker

matplotlib.rcParams.update({
    "font.sans-serif": ["Microsoft YaHei", "SimHei", "DejaVu Sans"],
    "axes.unicode_minus": False,
    "axes.formatter.use_mathtext": False,
})
import matplotlib.pyplot as plt

warnings.filterwarnings(
    "ignore",
    message=r"Glyph .* missing from font",
)
warnings.filterwarnings(
    "ignore",
    message=r"Font 'default' does not have a glyph for '\\u2212'",
)


class AsciiMinusLogFormatter(ticker.LogFormatterSciNotation):
    """对数坐标刻度：用 ASCII 减号替代 U+2212，避免中文字体缺字警告。"""

    def __call__(self, x, pos=None):
        label = super().__call__(x, pos)
        return label.replace("\u2212", "-") if isinstance(label, str) else label


def apply_log_scale(ax, log_x=False, log_y=False):
    if log_x:
        ax.set_xscale("log")
        ax.xaxis.set_major_formatter(AsciiMinusLogFormatter())
    if log_y:
        ax.set_yscale("log")
        ax.yaxis.set_major_formatter(AsciiMinusLogFormatter())

CAPACITIES = [10000, 100000, 1000000]
SMALL_CAPACITIES = [500]
ALGO_NAMES = ["贪心法", "动态规划", "回溯法", "蛮力法"]
FAST_ALGOS = ["贪心法", "动态规划", "回溯法"]
COLORS = ["#2ecc71", "#3498db", "#9b59b6", "#e74c3c"]
MARKERS = ["^", "s", "D", "o"]
LOG_Y_FLOOR_MS = 1e-4  # 对数坐标绘图下限，避免 log(0)


def load_records(csv_path: str):
    records = []
    with open(csv_path, encoding="utf-8-sig") as f:
        reader = csv.DictReader(f)
        for row in reader:
            if row.get("是否跳过", "").strip() == "是":
                continue
            try:
                records.append({
                    "capacity": int(row["背包容量"]),
                    "n_items": int(row["物品数量"]),
                    "algorithm": row["算法"].strip(),
                    "elapsed_ms": float(row["执行时间(ms)"]),
                    "space_bytes": int(float(row["空间占用(字节)"] or 0)),
                    "total_value": float(row["总价值"]) if row["总价值"] else 0,
                })
            except (ValueError, KeyError):
                continue
    return records


def algo_style(algo: str):
    idx = ALGO_NAMES.index(algo)
    return MARKERS[idx], COLORS[idx]


def plot_algo_time(ax, records, algos, capacities=None, log_y=False, y_floor=None):
    """在单个子图上绘制指定算法的耗时曲线。"""
    if capacities is None:
        capacities = [None]

    for algo in algos:
        marker, color = algo_style(algo)
        for capacity in capacities:
            cap_recs = records if capacity is None else [
                r for r in records if r["capacity"] == capacity
            ]
            algo_recs = sorted(
                [r for r in cap_recs if r["algorithm"] == algo],
                key=lambda x: x["n_items"],
            )
            if not algo_recs:
                continue
            xs = [r["n_items"] for r in algo_recs]
            ys = [r["elapsed_ms"] for r in algo_recs]
            if y_floor is not None:
                ys = [max(y, y_floor) for y in ys]
            label = algo if capacity is None else f"{algo} (W={capacity})"
            ax.plot(xs, ys, f"{marker}-", color=color,
                    label=label, linewidth=2, markersize=6)

    if log_y:
        apply_log_scale(ax, log_y=True)


def plot_time(records, output_dir):
    for capacity in CAPACITIES:
        fig, ax = plt.subplots(figsize=(12, 6))
        cap_recs = [r for r in records if r["capacity"] == capacity]

        for i, algo in enumerate(ALGO_NAMES):
            algo_recs = sorted(
                [r for r in cap_recs if r["algorithm"] == algo],
                key=lambda x: x["n_items"],
            )
            if not algo_recs:
                continue
            xs = [r["n_items"] for r in algo_recs]
            ys = [r["elapsed_ms"] for r in algo_recs]
            ax.plot(xs, ys, f"{MARKERS[i]}-", color=COLORS[i],
                    label=algo, linewidth=2, markersize=6)

        ax.set_xlabel("物品数量 n", fontsize=12)
        ax.set_ylabel("执行时间 (ms)", fontsize=12)
        ax.set_title(f"0-1 背包算法执行时间对比 (W={capacity})", fontsize=14)
        ax.legend(fontsize=10)
        ax.grid(True, alpha=0.3)
        apply_log_scale(ax, log_x=True, log_y=True)

        path = os.path.join(output_dir, f"time_comparison_W{capacity}.png")
        fig.tight_layout()
        fig.savefig(path, dpi=150)
        plt.close(fig)
        print(f"已保存: {path}")

    fig, axes = plt.subplots(1, 3, figsize=(18, 5))
    for ax_idx, capacity in enumerate(CAPACITIES):
        ax = axes[ax_idx]
        cap_recs = [r for r in records if r["capacity"] == capacity]
        for i, algo in enumerate(ALGO_NAMES):
            algo_recs = sorted(
                [r for r in cap_recs if r["algorithm"] == algo],
                key=lambda x: x["n_items"],
            )
            if not algo_recs:
                continue
            xs = [r["n_items"] for r in algo_recs]
            ys = [r["elapsed_ms"] for r in algo_recs]
            ax.plot(xs, ys, f"{MARKERS[i]}-", color=COLORS[i], label=algo, linewidth=2)
        ax.set_xlabel("物品数量 n")
        ax.set_ylabel("执行时间 (ms)")
        ax.set_title(f"W = {capacity}")
        ax.legend(fontsize=8)
        ax.grid(True, alpha=0.3)
        apply_log_scale(ax, log_x=True, log_y=True)

    path = os.path.join(output_dir, "time_comparison_all.png")
    fig.suptitle("0-1 背包 — 四种算法执行时间对比（对数坐标）", fontsize=14)
    fig.tight_layout()
    fig.savefig(path, dpi=150)
    plt.close(fig)
    print(f"已保存: {path}")


def plot_small_scale(records, output_dir):
    fig, axes = plt.subplots(1, 3, figsize=(16, 5))

    # 左：贪心 / 动态规划 / 回溯 — 线性坐标，便于看清毫秒级差异
    ax = axes[0]
    plot_algo_time(ax, records, FAST_ALGOS, capacities=SMALL_CAPACITIES, log_y=False)
    ax.set_xlabel("物品数量 n", fontsize=12)
    ax.set_ylabel("执行时间 (ms)", fontsize=12)
    ax.set_title("快速算法对比（线性坐标）", fontsize=13)
    ax.legend(fontsize=9)
    ax.grid(True, alpha=0.3)
    fast_times = [
        r["elapsed_ms"] for r in records
        if r["algorithm"] in FAST_ALGOS and r["elapsed_ms"] > 0
    ]
    if fast_times:
        ax.set_ylim(0, max(fast_times) * 1.25)

    # 中：蛮力法 — 指数增长，单独用对数坐标
    ax = axes[1]
    plot_algo_time(ax, records, ["蛮力法"], capacities=SMALL_CAPACITIES,
                   log_y=True, y_floor=LOG_Y_FLOOR_MS)
    ax.set_xlabel("物品数量 n", fontsize=12)
    ax.set_ylabel("执行时间 (ms)", fontsize=12)
    ax.set_title("蛮力法（对数坐标）", fontsize=13)
    ax.legend(fontsize=9)
    ax.grid(True, alpha=0.3)

    # 右：空间占用
    ax = axes[2]
    for algo in ALGO_NAMES:
        marker, color = algo_style(algo)
        algo_recs = sorted(
            [r for r in records if r["algorithm"] == algo],
            key=lambda x: x["n_items"],
        )
        if not algo_recs:
            continue
        xs = [r["n_items"] for r in algo_recs]
        ys = [r["space_bytes"] / 1024 for r in algo_recs]
        ax.plot(xs, ys, f"{marker}-", color=color,
                label=algo, linewidth=2, markersize=6)
    ax.set_xlabel("物品数量 n", fontsize=12)
    ax.set_ylabel("空间占用 (KB)", fontsize=12)
    ax.set_title("空间占用对比", fontsize=13)
    ax.legend(fontsize=9)
    ax.grid(True, alpha=0.3)

    path = os.path.join(output_dir, "small_scale_comparison.png")
    fig.suptitle("0-1 背包 — 小规模四算法完整对比 (W=500, n=10~28)", fontsize=14)
    fig.tight_layout()
    fig.savefig(path, dpi=150)
    plt.close(fig)
    print(f"已保存: {path}")


def plot_space(records, output_dir):
    fig, axes = plt.subplots(1, 3, figsize=(18, 5))
    for ax_idx, capacity in enumerate(CAPACITIES):
        ax = axes[ax_idx]
        cap_recs = [r for r in records if r["capacity"] == capacity]
        for i, algo in enumerate(ALGO_NAMES):
            algo_recs = sorted(
                [r for r in cap_recs if r["algorithm"] == algo],
                key=lambda x: x["n_items"],
            )
            if not algo_recs:
                continue
            xs = [r["n_items"] for r in algo_recs]
            ys = [r["space_bytes"] / (1024 * 1024) for r in algo_recs]
            ax.plot(xs, ys, "o-", color=COLORS[i], label=algo, linewidth=2)
        ax.set_xlabel("物品数量 n")
        ax.set_ylabel("空间占用 (MB)")
        ax.set_title(f"W = {capacity}")
        ax.legend(fontsize=8)
        ax.grid(True, alpha=0.3)
        apply_log_scale(ax, log_x=True)

    path = os.path.join(output_dir, "space_comparison_all.png")
    fig.suptitle("0-1 背包 — 空间占用对比", fontsize=14)
    fig.tight_layout()
    fig.savefig(path, dpi=150)
    plt.close(fig)
    print(f"已保存: {path}")


def plot_csv(csv_path: str):
    output_dir = os.path.dirname(csv_path) or os.path.dirname(os.path.abspath(__file__))
    if not os.path.isfile(csv_path):
        print(f"找不到 CSV: {csv_path}")
        return False

    records = load_records(csv_path)
    if not records:
        print(f"CSV 中无有效记录: {csv_path}")
        return False

    print(f"读取 {len(records)} 条有效记录: {csv_path}")
    if "small_scale" in os.path.basename(csv_path):
        plot_small_scale(records, output_dir)
    else:
        plot_time(records, output_dir)
        plot_space(records, output_dir)
    return True


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    output_dir = os.path.join(script_dir, "output")

    if len(sys.argv) > 1:
        if not plot_csv(sys.argv[1]):
            sys.exit(1)
        print("绘图完成。")
        return

    large_csv = os.path.join(output_dir, "experiment_results.csv")
    small_csv = os.path.join(output_dir, "small_scale_results.csv")
    plotted = False

    if plot_csv(small_csv):
        plotted = True
    if plot_csv(large_csv):
        plotted = True

    if not plotted:
        print("找不到实验结果 CSV，请先运行 run_experiment.exe 或 build.bat")
        sys.exit(1)

    print("绘图完成。")


if __name__ == "__main__":
    main()
