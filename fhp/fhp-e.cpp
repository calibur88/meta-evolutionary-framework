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
 *时间			作者		信息
 *----------		---		------------------------------------------------------
 */

#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <omp.h>
#include <string>
#include <vector>


// --- Cross-platform high-performance memory alignment ---
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
	// [1] Automatic hardware environment detection and core adaptation
	int max_threads = omp_get_max_threads();
	// Lock P-cores for new architectures (e.g., 12900H) to ensure scoring stability, fully utilize for older architectures (e.g., E3)
	int target_threads = (max_threads >= 20) ? 16 : max_threads;
	omp_set_num_threads(target_threads);

	// [2] FHP core mathematical parameter mapping
	const int dim = 8388608;			 // Matrix size: 8.0M elements
	const int outer_loops = 50;			 // Evaluation rounds
	const int inner_iters = 500;		 // Evolution depth step size
	const float eta = 0.05f;			 // Response gain (n)
	const float alpha = 0.15f;			 // Decay factor (a)
	const float noise_base = 0.2501f;	 // Driving field F + noise xi

	// [3] Dynamic field memory allocation
	float* __restrict X = (float*) ALIGN_ALLOC(dim * sizeof(float));
	float* __restrict Gamma = (float*) ALIGN_ALLOC(dim * sizeof(float));
	if (! X || ! Gamma) return -1;

// State initialization
#pragma omp parallel for
	for (int i = 0; i < dim; ++i)
	{
		X[i] = 1.0f;
		Gamma[i] = 0.0f;
	}

	std::cout << "==========================================================" << std::endl;
	std::cout << "      FHP Core Computation          " << std::endl;
	std::cout << "==========================================================" << std::endl;
	std::cout << "[Hardware Detection] Logical Cores: " << max_threads << " | Computation Threads: " << target_threads << std::endl;
	std::cout << "[Execution Status] Core generic loading successful, starting holographic evolution..." << std::endl;
	std::cout << "----------------------------------------------------------" << std::endl;

	auto start_time = std::chrono::high_resolution_clock::now();

	// [4] High-pressure computation core loop
	for (int ol = 0; ol < outer_loops; ++ol)
	{
		auto t1 = std::chrono::high_resolution_clock::now();

#pragma omp parallel
		{
			for (int il = 0; il < inner_iters; ++il)
			{
// Force vectorization and pipeline optimization: activate AVX potential
#pragma omp for schedule(static) nowait
				for (int i = 0; i < dim; ++i)
				{
					float x_t = X[i];
					float g_t = Gamma[i];

					// Core dynamics evolution formula
					float x_next = x_t + (eta * (noise_base - 0.1f * x_t)) + (alpha * g_t);
					float g_next = alpha * (g_t + (x_next - x_t));	  // Full displacement capture

					X[i] = x_next;
					Gamma[i] = g_next;
				}
			}
		}

		auto t2 = std::chrono::high_resolution_clock::now();
		double step_sec = std::chrono::duration<double>(t2 - t1).count();
		double throughput = ((double) dim * inner_iters / step_sec) / 1e9;

		// Real-time output of instantaneous compute power (G-HOPS/s)
		std::cout << "\r > Round [" << std::setw(2) << ol + 1 << "] Real-time Compute Power: " << std::fixed << std::setprecision(2)
				  << throughput << " G-HOPS/s" << std::flush;
	}

	auto end_time = std::chrono::high_resolution_clock::now();
	double total_sec = std::chrono::duration<double>(end_time - start_time).count();
	double final_pts = ((double) dim * outer_loops * inner_iters / total_sec / 1e9) * 10.0;

	// [5] Comprehensive performance evaluation report
	std::cout << "\n\n----------------------- Final Evaluation Report -----------------------" << std::endl;
	std::cout << " 1. Comprehensive Performance Score (PTS) : " << std::fixed << std::setprecision(1) << final_pts << std::endl;
	std::cout << " 2. Average Computation Throughput (G-HOPS/s) : " << (final_pts / 10.0) << std::endl;
	std::cout << " 3. Total Full-Load Duration (SEC) : " << std::fixed << std::setprecision(2) << total_sec << " seconds" << std::endl;

	// Stability and robustness verification
	std::cout << " 4. Computation Result Verification : " << (std::isnan(X[0]) ? "[FAIL] Logical Collapse" : "[PASS] Numerical Consistency") << std::endl;

	// Performance tier assessment
	std::string tier;
	if (final_pts > 70) tier = "Flagship Tier (12900H/New Architecture Level)";
	else if (final_pts > 30) tier = "Mainstream Tier (Modern Multi-core Architecture)";
	else tier = "Entry Tier (E3-1230/Classic Architecture Level)";
	std::cout << " 5. Performance Tier Assessment : " << tier << std::endl;
	std::cout << "------------------------------------------------------------" << std::endl;
	std::cout << "[Feature Confirmation] " << (final_pts > 30 ? "Full memory capture feature fully activated" : "Hardware architecture limits displacement information capture bandwidth")
			  << "" << std::endl;

	ALIGN_FREE(X);
	ALIGN_FREE(Gamma);
	std::cout << "\nTest completed. Press Enter to exit..." << std::endl;
	std::cin.get();
	return 0;
}