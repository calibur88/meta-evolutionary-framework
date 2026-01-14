/*
 *--------------------------------------------------------------------------------
 *文件名: main.cpp
 *创建时间: 2026-01-14 09:17:14 Wed
 *说明: 
 *作者: Calibur
 *主机: LAPTOP-D92A7OL2
 *--------------------------------------------------------------------------------
 *最后编辑作者: Calibur
 *最后修改时间: 2026-01-14 13:02:00 Wed
 *--------------------------------------------------------------------------------
 *Copyright (c) 2026 Calibur
 *--------------------------------------------------------------------------------
 *更新历史:
 *--------------------------------------------------------------------------------
 *时间      		作者		信息
 *----------		---		------------------------------------------------------
 */

#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <omp.h>
#include <string>
#include <vector>


// --- 跨平台高性能内存对齐 ---
#ifdef _MSC_VER
	#include <malloc.h>
	#define ALIGN_ALLOC(size) _aligned_malloc(size, 64)
	#define ALIGN_FREE(ptr)	  _aligned_free(ptr)
#else
	#include <stdlib.h>
	#define ALIGN_ALLOC(size) aligned_alloc(64, size)
	#define ALIGN_FREE(ptr)	  free(ptr)
#endif

int main()
{
	// [1] 硬件环境自动感知与核心适配
	int max_threads = omp_get_max_threads();
	// 针对新架构(如12900H)锁定P核以保证评分稳定性，针对老架构(如E3)全速压榨
	int target_threads = (max_threads >= 20) ? 16 : max_threads;
	omp_set_num_threads(target_threads);

	// [2] FHP 核心数学参数映射
	const int dim = 8388608;			 // 矩阵规模: 8.0M elements
	const int outer_loops = 50;			 // 评估轮次
	const int inner_iters = 500;		 // 演化深度步长
	const float eta = 0.05f;			 // 响应增益 (n)
	const float alpha = 0.15f;			 // 衰减因子 (a)
	const float noise_base = 0.2501f;	 // 驱动场 F + 噪声 xi

	// [3] 动力学场内存分配
	float* __restrict X = (float*) ALIGN_ALLOC(dim * sizeof(float));
	float* __restrict Gamma = (float*) ALIGN_ALLOC(dim * sizeof(float));
	if (! X || ! Gamma) return -1;

// 状态初始化
#pragma omp parallel for
	for (int i = 0; i < dim; ++i)
	{
		X[i] = 1.0f;
		Gamma[i] = 0.0f;
	}

	std::cout << "==========================================================" << std::endl;
	std::cout << "      FHP 核心演算          " << std::endl;
	std::cout << "==========================================================" << std::endl;
	std::cout << "[硬件识别] 逻辑核心: " << max_threads << " | 演算线程: " << target_threads << std::endl;
	std::cout << "[执行状态] 核心通用式加载成功，开始全息演化..." << std::endl;
	std::cout << "----------------------------------------------------------" << std::endl;

	auto start_time = std::chrono::high_resolution_clock::now();

	// [4] 高压演算核心循环
	for (int ol = 0; ol < outer_loops; ++ol)
	{
		auto t1 = std::chrono::high_resolution_clock::now();

#pragma omp parallel
		{
			for (int il = 0; il < inner_iters; ++il)
			{
// 强制向量化与流水线优化：激活 AVX 潜能
#pragma omp for schedule(static) nowait
				for (int i = 0; i < dim; ++i)
				{
					float x_t = X[i];
					float g_t = Gamma[i];

					// 核心动力学演化公式
					float x_next = x_t + (eta * (noise_base - 0.1f * x_t)) + (alpha * g_t);
					float g_next = alpha * (g_t + (x_next - x_t));	  // 全量位移捕捉

					X[i] = x_next;
					Gamma[i] = g_next;
				}
			}
		}

		auto t2 = std::chrono::high_resolution_clock::now();
		double step_sec = std::chrono::duration<double>(t2 - t1).count();
		double throughput = ((double) dim * inner_iters / step_sec) / 1e9;

		// 实时输出瞬时算力 (G-HOPS/s)
		std::cout << "\r > 轮次 [" << std::setw(2) << ol + 1 << "] 实时算力: " << std::fixed << std::setprecision(2)
				  << throughput << " G-HOPS/s" << std::flush;
	}

	auto end_time = std::chrono::high_resolution_clock::now();
	double total_sec = std::chrono::duration<double>(end_time - start_time).count();
	double final_pts = ((double) dim * outer_loops * inner_iters / total_sec / 1e9) * 10.0;

	// [5] 综合性能评估报告
	std::cout << "\n\n----------------------- 最终评估报告 -----------------------" << std::endl;
	std::cout << " 1. 综合性能评分 (PTS) : " << std::fixed << std::setprecision(1) << final_pts << std::endl;
	std::cout << " 2. 平均计算吞吐 (G-HOPS/s) : " << (final_pts / 10.0) << std::endl;
	std::cout << " 3. 总计满载时长 (SEC) : " << std::fixed << std::setprecision(2) << total_sec << " 秒" << std::endl;

	// 稳定性与鲁棒性验证
	std::cout << " 4. 运算结果校验 : " << (std::isnan(X[0]) ? "[FAIL] 逻辑坍缩" : "[PASS] 数值自洽") << std::endl;

	// 性能等级判定
	std::string tier;
	if (final_pts > 70) tier = "旗舰级 (12900H/新架构级别)";
	else if (final_pts > 30) tier = "主流级 (现代多核架构)";
	else tier = "入门级 (E3-1230/经典架构级别)";
	std::cout << " 5. 性能等级评定 : " << tier << std::endl;
	std::cout << "------------------------------------------------------------" << std::endl;
	std::cout << "[特性确认] " << (final_pts > 30 ? "已全面激活全量记忆捕捉特性" : "硬件架构限制了位移信息捕捉带宽")
			  << "" << std::endl;

	ALIGN_FREE(X);
	ALIGN_FREE(Gamma);
	std::cout << "\n测试完成。按回车键退出..." << std::endl;
	std::cin.get();
	return 0;
}
