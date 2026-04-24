/**
 * Scientific Math Library Implementation
 */

#include "scientific_math.h"

// 全局角度模式（默认角度制）
AngleMode g_angleMode = DEGREE;

// ============================================================
// 角度/弧度转换
// ============================================================
double toRadians(double angle) {
  if (g_angleMode == DEGREE) {
    return DEG_TO_RAD(angle);
  }
  return angle;  // 已经是弧度
}

double fromRadians(double angle) {
  if (g_angleMode == DEGREE) {
    return RAD_TO_DEG(angle);
  }
  return angle;
}

// ============================================================
// 三角函数
// ============================================================
double calc_sin(double x) {
  return sin(toRadians(x));
}

double calc_cos(double x) {
  return cos(toRadians(x));
}

double calc_tan(double x) {
  double rad = toRadians(x);
  // 检查 cos 是否接近零（避免除零）
  if (fabs(cos(rad)) < CALC_EPSILON) {
    return NAN;  // 返回非数字表示错误
  }
  return tan(rad);
}

double calc_asin(double x) {
  if (x < -1.0 || x > 1.0) return NAN;
  return fromRadians(asin(x));
}

double calc_acos(double x) {
  if (x < -1.0 || x > 1.0) return NAN;
  return fromRadians(acos(x));
}

double calc_atan(double x) {
  return fromRadians(atan(x));
}

// ============================================================
// 双曲函数
// ============================================================
double calc_sinh(double x) {
  return sinh(x);
}

double calc_cosh(double x) {
  return cosh(x);
}

double calc_tanh(double x) {
  return tanh(x);
}

double calc_asinh(double x) {
  return log(x + sqrt(x * x + 1.0));
}

double calc_acosh(double x) {
  if (x < 1.0) return NAN;
  return log(x + sqrt(x * x - 1.0));
}

double calc_atanh(double x) {
  if (x <= -1.0 || x >= 1.0) return NAN;
  return 0.5 * log((1.0 + x) / (1.0 - x));
}

// ============================================================
// 对数与指数
// ============================================================
double calc_ln(double x) {
  if (x <= 0) return NAN;
  return log(x);
}

double calc_log(double x) {
  if (x <= 0) return NAN;
  return log10(x);
}

double calc_log2(double x) {
  if (x <= 0) return NAN;
  return log(x) / log(2.0);
}

double calc_exp(double x) {
  return exp(x);
}

double calc_pow(double base, double exp) {
  // 处理特殊情况
  if (base == 0.0 && exp <= 0.0) return NAN;  // 0 的负次方或 0^0
  if (base < 0.0 && !isInteger(exp)) return NAN;  // 负数的非整数次方
  return pow(base, exp);
}

double calc_sqrt(double x) {
  if (x < 0) return NAN;
  return sqrt(x);
}

double calc_cbrt(double x) {
  return cbrt(x);
}

double calc_nthRoot(double x, double n) {
  if (n == 0) return NAN;
  if (x < 0 && isInteger(n) && ((long)n % 2 == 0)) return NAN;  // 负数偶次方根
  if (x < 0) {
    // 奇次方根
    return -pow(-x, 1.0 / n);
  }
  return pow(x, 1.0 / n);
}

// ============================================================
// 其他数学函数
// ============================================================
double calc_abs(double x) {
  return fabs(x);
}

double calc_floor(double x) {
  return floor(x);
}

double calc_ceil(double x) {
  return ceil(x);
}

double calc_round(double x) {
  return round(x);
}

// 伽马函数近似（Lanczos近似，用于小数阶乘）
double calc_gamma(double x) {
  if (x <= 0 && isInteger(x)) return NAN;  // 负整数无定义
  
  // 使用 Lanczos 近似
  const double p[] = {
    676.5203681218851,
    -1259.1392167224028,
    771.32342877765313,
    -176.61502916214059,
    12.507343278686905,
    -0.13857109526572012,
    9.9843695780195716e-6,
    1.5056327351493116e-7
  };
  
  int g = 7;
  if (x < 0.5) {
    return CALC_PI / (sin(CALC_PI * x) * calc_gamma(1.0 - x));
  }
  
  x -= 1.0;
  double a = 0.99999999999980993;
  for (int i = 0; i < 8; i++) {
    a += p[i] / (x + (double)(i + 1));
  }
  
  double t = x + (double)g + 0.5;
  return sqrt(2.0 * CALC_PI) * pow(t, x + 0.5) * exp(-t) * a;
}

double calc_factorial(double x) {
  if (x < 0) return NAN;
  if (isInteger(x)) {
    // 整数阶乘
    long n = (long)round(x);
    if (n > 170) return INFINITY;  // 溢出
    if (n == 0 || n == 1) return 1.0;
    double result = 1.0;
    for (long i = 2; i <= n; i++) {
      result *= (double)i;
    }
    return result;
  } else {
    // 小数阶乘: n! = Gamma(n+1)
    return calc_gamma(x + 1.0);
  }
}

double calc_mod(double a, double b) {
  if (b == 0) return NAN;
  return fmod(a, b);
}

// ============================================================
// 统计函数
// ============================================================
double calc_sum(const double* data, uint8_t count) {
  double sum = 0.0;
  for (uint8_t i = 0; i < count; i++) {
    sum += data[i];
  }
  return sum;
}

double calc_mean(const double* data, uint8_t count) {
  if (count == 0) return NAN;
  return calc_sum(data, count) / count;
}

double calc_stdDev(const double* data, uint8_t count) {
  if (count < 2) return NAN;
  double mean = calc_mean(data, count);
  double sumSq = 0.0;
  for (uint8_t i = 0; i < count; i++) {
    double diff = data[i] - mean;
    sumSq += diff * diff;
  }
  return sqrt(sumSq / (count - 1));  // 样本标准差
}

// ============================================================
// 数论函数
// ============================================================
long calc_gcd(long a, long b) {
  a = labs(a);
  b = labs(b);
  while (b != 0) {
    long temp = b;
    b = a % b;
    a = temp;
  }
  return a;
}

long calc_lcm(long a, long b) {
  if (a == 0 || b == 0) return 0;
  return labs(a * b) / calc_gcd(a, b);
}

bool calc_isPrime(long n) {
  if (n < 2) return false;
  if (n == 2) return true;
  if (n % 2 == 0) return false;
  for (long i = 3; i * i <= n; i += 2) {
    if (n % i == 0) return false;
  }
  return true;
}

// ============================================================
// 组合数学
// ============================================================
double calc_permutation(double n, double r) {
  if (n < 0 || r < 0 || r > n) return NAN;
  if (!isInteger(n) || !isInteger(r)) return NAN;
  long nl = (long)round(n);
  long rl = (long)round(r);
  double result = 1.0;
  for (long i = 0; i < rl; i++) {
    result *= (double)(nl - i);
  }
  return result;
}

double calc_combination(double n, double r) {
  if (n < 0 || r < 0 || r > n) return NAN;
  if (!isInteger(n) || !isInteger(r)) return NAN;
  long nl = (long)round(n);
  long rl = (long)round(r);
  // C(n,r) = C(n, n-r)，取较小的计算
  if (rl > nl - rl) rl = nl - rl;
  double result = 1.0;
  for (long i = 0; i < rl; i++) {
    result = result * (nl - i) / (i + 1);
  }
  return result;
}

// ============================================================
// 工具函数
// ============================================================
bool isEqual(double a, double b, double epsilon) {
  return fabs(a - b) < epsilon;
}

bool isInteger(double x) {
  return fabs(x - round(x)) < CALC_EPSILON;
}

double sign(double x) {
  if (x > 0) return 1.0;
  if (x < 0) return -1.0;
  return 0.0;
}
