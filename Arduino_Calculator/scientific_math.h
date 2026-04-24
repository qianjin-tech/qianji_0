/**
 * Scientific Math Library for Arduino Calculator
 * 提供超越基础四则运算的科学计算函数
 */

#ifndef SCIENTIFIC_MATH_H
#define SCIENTIFIC_MATH_H

#include "config.h"
#include <math.h>

// ============================================================
// 角度/弧度转换（根据当前模式）
// ============================================================
extern AngleMode g_angleMode;

double toRadians(double angle);
double fromRadians(double angle);

// ============================================================
// 三角函数（自动处理角度/弧度模式）
// ============================================================
double calc_sin(double x);
double calc_cos(double x);
double calc_tan(double x);
double calc_asin(double x);
double calc_acos(double x);
double calc_atan(double x);

// ============================================================
// 双曲函数
// ============================================================
double calc_sinh(double x);
double calc_cosh(double x);
double calc_tanh(double x);
double calc_asinh(double x);
double calc_acosh(double x);
double calc_atanh(double x);

// ============================================================
// 对数与指数
// ============================================================
double calc_ln(double x);       // 自然对数
double calc_log(double x);      // 常用对数（以10为底）
double calc_log2(double x);     // 以2为底的对数
double calc_exp(double x);      // e^x
double calc_pow(double base, double exp);  // 幂运算
double calc_sqrt(double x);     // 平方根
double calc_cbrt(double x);     // 立方根
double calc_nthRoot(double x, double n);   // n次方根

// ============================================================
// 其他数学函数
// ============================================================
double calc_abs(double x);
double calc_floor(double x);
double calc_ceil(double x);
double calc_round(double x);
double calc_factorial(double x);    // 阶乘（支持小数用伽马函数近似）
double calc_gamma(double x);        // 伽马函数（用于小数阶乘）
double calc_mod(double a, double b); // 取模运算

// ============================================================
// 统计函数
// ============================================================
double calc_sum(const double* data, uint8_t count);
double calc_mean(const double* data, uint8_t count);
double calc_stdDev(const double* data, uint8_t count);  // 标准差

// ============================================================
// 数论函数
// ============================================================
long calc_gcd(long a, long b);      // 最大公约数
long calc_lcm(long a, long b);      // 最小公倍数
bool calc_isPrime(long n);          // 素数判断

// ============================================================
// 组合数学
// ============================================================
double calc_permutation(double n, double r);  // 排列数 P(n,r)
double calc_combination(double n, double r);  // 组合数 C(n,r)

// ============================================================
// 工具函数
// ============================================================
bool isEqual(double a, double b, double epsilon = CALC_EPSILON);
bool isInteger(double x);
double sign(double x);

#endif // SCIENTIFIC_MATH_H
