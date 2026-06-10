@echo off
chcp 65001 >nul
echo 编译排序算法对比实验 (C 语言)...

where gcc >nul 2>&1
if %ERRORLEVEL%==0 (
    gcc -Wall -Wextra -O2 -std=c99 -o run_experiment.exe ^
        common.c data_generator.c bubble_sort.c merge_sort.c ^
        quick_sort.c run_experiment.c
    if %ERRORLEVEL%==0 (
        echo 编译成功！开始运行实验...
        run_experiment.exe
        echo.
        echo 生成图表...
        python plot_results.py
    )
    goto :end
)

echo [错误] 未找到 gcc，请安装 MinGW-w64 / MSYS2 后重试。

:end
