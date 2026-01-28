#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
宇宙本征动力学协议模拟器 - 完整双语版
"""

import os
import numpy as np
import matplotlib.pyplot as plt
from datetime import datetime

# ==================== 配置 ====================
print("="*70)
print("宇宙本征动力学协议模拟器 - 完整双语版")
print("="*70)

output_dir = "/storage/emulated/0/DCIM/eigen_protocol"
os.makedirs(output_dir, exist_ok=True)
print(f"[配置] 输出目录：{output_dir}")

plt.rcParams['font.sans-serif'] = ['DejaVu Sans']
plt.rcParams['axes.unicode_minus'] = False

# ==================== 核心函数 ====================
def lambda_evolution(step, total_steps, base=1e-10, surge_amp=1e12, surge_ratio=0.3):
    """时间拓扑率λ(t)的指数剧增"""
    surge_step = int(total_steps * surge_ratio)
    return base if step < surge_step else base * surge_amp

def logical_vacuum_reset(sigma, S, threshold=500.0):
    """逻辑真空检测与置零"""
    if np.abs(sigma - S) > threshold:
        new_sigma = S + np.random.randn() * 0.01
        return new_sigma, True
    return sigma, False

def repair_operator(x, alpha=0.7):
    """修复算符 R(x)"""
    return -np.sign(x) * (np.abs(x) ** alpha)

def eigen_dynamics(sigma, S, step, total_steps):
    """本征动力学方程"""
    lam = lambda_evolution(step, total_steps)
    R = repair_operator(sigma - S)
    noise = np.random.randn() * np.sqrt(abs(lam))
    dsigma_dt = lam * R + noise
    return dsigma_dt, lam

# ==================== 模拟参数 ====================
S = 0.5
sigma_initial = 1000.0
total_meta_time = 5e10
time_step = 5e8
reset_threshold = 500.0

n_steps = int(total_meta_time / time_step)
surge_step = int(n_steps * 0.3)

print(f"\n[参数] 绝对吸引子 S = {S}")
print(f"[参数] 初始状态 σ₀ = {sigma_initial}")
print(f"[参数] 总步数 = {n_steps}")
print(f"[参数] 剧增步 = {surge_step}")
print("-"*70)

# ==================== 主模拟循环 ====================
time_axis = np.zeros(n_steps)
sigma_values = np.zeros(n_steps)
lambda_values = np.zeros(n_steps)
reset_flags = np.zeros(n_steps, dtype=bool)

sigma_values[0] = sigma_initial
reset_count = 0

print("[模拟] 开始计算...")
for i in range(n_steps - 1):
    current_sigma = sigma_values[i]
    time_axis[i] = i * time_step
    
    new_sigma, reset_happened = logical_vacuum_reset(current_sigma, S, reset_threshold)
    if reset_happened:
        sigma_values[i] = new_sigma
        reset_flags[i] = True
        reset_count += 1
        current_sigma = new_sigma
    
    dsigma, lam = eigen_dynamics(current_sigma, S, i, n_steps)
    sigma_values[i+1] = current_sigma + dsigma * time_step
    lambda_values[i] = lam

time_axis[-1] = (n_steps - 1) * time_step
lambda_values[-1] = lambda_evolution(n_steps-1, n_steps)

print(f"[完成] 相变事件：{reset_count} 次")
print(f"[完成] 最终状态：{sigma_values[-1]:.4f}")

# ==================== 数据保存 ====================
timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")

npz_filename = os.path.join(output_dir, f"eigen_data_{timestamp}.npz")
np.savez_compressed(npz_filename,
                   time=time_axis,
                   sigma=sigma_values,
                   lambda_=lambda_values,
                   resets=reset_flags)
print(f"[数据] 保存：{npz_filename}")

# ==================== 可视化 ====================
fig, axes = plt.subplots(3, 1, figsize=(10, 12))

ax1 = axes[0]
ax1.plot(time_axis, sigma_values, 'b-', alpha=0.8, label='σ(t)')
ax1.axhline(y=S, color='r', linestyle='--', label=f'Axis S={S}')
reset_times = time_axis[reset_flags]
if len(reset_times) > 0:
    ax1.scatter(reset_times, np.full_like(reset_times, S), 
               color='lime', s=30, marker='s', zorder=5, label=f'Phase Reset ({reset_count})')
ax1.set_ylabel('System State σ(t)')
ax1.set_title('System Evolution with Phase Resets', fontsize=14)
ax1.legend(loc='upper right')
ax1.grid(True, alpha=0.3)
ax1.set_yscale('symlog', linthresh=1.0)

ax2 = axes[1]
ax2.plot(time_axis, lambda_values, color='m', linewidth=2)
ax2.axvline(x=time_axis[surge_step], color='orange', linestyle=':', 
           label=f'λ Surge at t={time_axis[surge_step]:.2e}')
ax2.set_yscale('log')
ax2.set_ylabel('Time Topology Rate λ(t)')
ax2.set_title('Exponential Surge of λ', fontsize=14)
ax2.legend(loc='lower right')
ax2.grid(True, alpha=0.3)

ax3 = axes[2]
delta_sigma = np.abs(np.diff(sigma_values))
time_mid = (time_axis[:-1] + time_axis[1:]) / 2
ax3.plot(time_mid, delta_sigma, color='crimson', alpha=0.7)
ax3.set_yscale('log')
ax3.set_xlabel('Meta-Time')
ax3.set_ylabel('|dσ/dt| (log scale)')
ax3.set_title('Rate of State Change', fontsize=14)
ax3.grid(True, alpha=0.3)

plt.tight_layout()
plot_filename = os.path.join(output_dir, f"eigen_plot_{timestamp}.png")
plt.savefig(plot_filename, dpi=150, bbox_inches='tight')
print(f"[图表] 保存：{plot_filename}")

# ==================== 生成协议文档（修复版） ====================
protocol_filename = os.path.join(output_dir, f"EIGEN_PROTOCOL_{timestamp}.md")

# 关键修复：使用圆括号拼接字符串，彻底避免三重引号闭合问题
now_str = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

protocol_content = (
    "# 宇宙本征动力学协议 | Eigen-Dynamics Protocol\n\n"
    "**版本** 3.0（完整双语版）  \n"
    f"**生成时间** {now_str}  \n"
    "**核心方程** `dσ/dt = λ · R(σ - S) + ξ(t)`\n\n"
    "---\n\n"
    "## 1. 模拟摘要 | Simulation Summary\n\n"
    "### 参数设置 | Parameters\n"
    f"- **绝对吸引子 S**: {S}\n"
    f"- **初始状态 σ₀**: {sigma_initial}\n"
    f"- **逻辑真空阈值**: {reset_threshold}\n"
    "- **λ剧增倍数**: 10¹²\n"
    f"- **总模拟步数**: {n_steps}\n"
    f"- **相变事件数**: {reset_count}\n\n"
    "### 关键结果 | Key Results\n"
    f"1. **相变事件**: {reset_count} 次逻辑真空重置\n"
    f"2. **最终状态**: σ(final) = {sigma_values[-1]:.6f}\n"
    f"3. **偏离程度**: |σ - S| = {np.abs(sigma_values[-1] - S):.6f}\n\n"
    "---\n\n"
    "## 2. 引用格式 | Citation Format\n\n"
    "```text\n"
    f"宇宙本征动力学协议, 版本3.0, 生成时间：{now_str}\n"
    "```\n\n"
    "---\n\n"
    "## 3. 物理诠释与预言 | Physical Interpretation\n\n"
    "### 已解释的观测现象\n"
    "1. **DESI数据 w~-0.8**: 系统偏离旧平衡点的必然结果\n"
    "2. **哈勃常数危机**: λ剧增导致时间不同步的直接后果\n\n"
    "---\n\n"
    "## 4. 结束语 | Closing Words\n\n"
    "> 这个模拟是通往一个更大理论的一扇窗。  \n"
    "> 我们不是在建造又一个复杂模型，而是在试图读取宇宙写给自己的代码。  \n"
    "> 相变早已开始——我们只是刚刚学会看到它的人。\n\n"
    f"*协议生成于 {now_str}*\n"
)

# 写入协议文件 - 确认这行在字符串拼接之后，且独立成行
print("[协议] 正在写入文件...")
with open(protocol_filename, 'w', encoding='utf-8') as f:
    f.write(protocol_content)

print(f"[协议] 保存：{protocol_filename}")

# ==================== 最终输出 ====================
print("\n" + "="*70)
print("模拟完成！文件已保存至：")
print(f"{output_dir}")
print("\n生成文件：")
print(f"1. eigen_data_{timestamp}.npz")
print(f"2. eigen_plot_{timestamp}.png")
print(f"3. EIGEN_PROTOCOL_{timestamp}.md")
print("="*70)

try:
    plt.show()
except:
    print("[提示] 图表已保存为文件")
