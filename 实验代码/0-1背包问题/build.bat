@echo off
chcp 65001 >nul
echo 编译 0-1 背包问题 C 语言实验（无规模限制）...

where gcc >nul 2>&1
if %ERRORLEVEL%==0 (
    gcc -Wall -Wextra -O2 -std=c99 -o run_experiment.exe ^
        common.c data_generator.c brute_force.c dynamic_programming.c ^
        greedy.c backtracking.c export_data.c run_experiment.c -lm
    if %ERRORLEVEL%==0 (
        echo 编译成功！开始运行实验...
        run_experiment.exe
        echo.
        echo 生成图表（大规模 + 小规模四算法对比）...
        python plot_results.py
    )
    goto :end
)

echo [错误] 未找到 gcc，请安装 MinGW-w64 后重试。

:end
