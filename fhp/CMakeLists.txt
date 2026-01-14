cmake_minimum_required(VERSION 3.15)
project(FHP_Benchmark_Universal LANGUAGES CXX)

# 1. 编译标准设定
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. 查找 OpenMP 并行库 (实现全核心满载的核心)
find_package(OpenMP REQUIRED)

# 3. 定义可执行文件
add_executable(fhp_bench fhp-c.cpp)

# 4. 针对 Windows (MSVC) 的深度性能与兼容性优化
if(MSVC)
    # --- 策略 A: 强制全平台静态链接 (/MT) ---
    # 确保 EXE 不依赖 VCRUNTIME140.dll，可在任何 Windows 机器（如 E3 机器）直接双击运行
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
    
    # 强制覆盖 CMake 默认的 /MD (动态链接) 标志
    foreach(flag_var 
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE 
        CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL)
        if(${flag_var} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endif()
    endforeach()

    # --- 策略 B: 跨架构性能对标开关 ---
    # /O2: 速度最大化
    # /fp:fast: 允许浮点重排，这是开启 AVX 向量化并冲破 E3 评分瓶颈的关键
    # /arch:AVX: 锁定为 E3 支持的指令集，防止在老机器上出现“非法指令”错误
    # /GL: 开启全程序优化 (Whole Program Optimization)
    target_compile_options(fhp_bench PRIVATE 
        /O2 /Oi /Ot /Oy /GL /fp:fast /openmp /arch:AVX /W3
    )

    # --- 策略 C: 链接器深度优化 ---
    # /LTCG: 链接时代码生成，进一步压缩逻辑指令
    target_link_options(fhp_bench PRIVATE /LTCG /OPT:REF /OPT:ICF)

else()
    # 5. 针对 Linux (GCC/Clang) 的优化
    # -static: 静态链接系统库
    # -mavx: 明确指定 AVX 指令集
    target_compile_options(fhp_bench PRIVATE -O3 -ffast-math -fopenmp -mavx -static)
endif()

# 6. 绑定并行库
target_link_libraries(fhp_bench PUBLIC OpenMP::OpenMP_CXX)

# 7. 终端状态反馈
message(STATUS "------------------------------------------------")
message(STATUS " [FHP 跨平台对标配置完成] ")
message(STATUS " 编译模式: Release")
message(STATUS " 链接模式: Static (/MT) - 独立运行")
message(STATUS " 优化目标: AVX 向量化 (兼容新老 CPU)")
message(STATUS "------------------------------------------------")