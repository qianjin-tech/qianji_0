/**
 * Complex Number Library for Arduino Calculator
 * 支持复数的四则运算、模、辐角、共轭等操作
 */

#ifndef COMPLEX_NUMBER_H
#define COMPLEX_NUMBER_H

#include "config.h"
#include <math.h>

// ============================================================
// 复数结构体
// ============================================================
struct Complex {
  double real;    // 实部
  double imag;    // 虚部

  // 构造函数
  Complex(double r = 0.0, double i = 0.0) : real(r), imag(i) {}

  // 判断是否为纯实数
  bool isReal() const {
    return fabs(imag) < CALC_EPSILON;
  }

  // 判断是否为纯虚数
  bool isImaginary() const {
    return fabs(real) < CALC_EPSILON && fabs(imag) >= CALC_EPSILON;
  }

  // 判断是否为零
  bool isZero() const {
    return fabs(real) < CALC_EPSILON && fabs(imag) < CALC_EPSILON;
  }

  // 格式化输出到字符串
  void toString(char* buffer, uint8_t len, uint8_t precision = DISPLAY_PRECISION) const;
};

// ============================================================
// 复数基本运算
// ============================================================
Complex cAdd(const Complex& a, const Complex& b);
Complex cSub(const Complex& a, const Complex& b);
Complex cMul(const Complex& a, const Complex& b);
Complex cDiv(const Complex& a, const Complex& b);

// ============================================================
// 复数属性
// ============================================================
double cAbs(const Complex& z);       // 模 |z|
double cArg(const Complex& z);       // 辐角 arg(z)（弧度）
Complex cConj(const Complex& z);     // 共轭复数

// ============================================================
// 复数科学函数
// ============================================================
Complex cExp(const Complex& z);      // e^z
Complex cLn(const Complex& z);       // ln(z)
Complex cPow(const Complex& base, const Complex& exp);  // z1^z2
Complex cSqrt(const Complex& z);     // sqrt(z)

// ============================================================
// 复数三角函数
// ============================================================
Complex cSin(const Complex& z);
Complex cCos(const Complex& z);
Complex cTan(const Complex& z);

// ============================================================
// 极坐标/直角坐标转换
// ============================================================
Complex polarToRect(double r, double theta);  // (r, θ) -> (a+bi)
void rectToPolar(const Complex& z, double& r, double& theta);  // (a+bi) -> (r, θ)

// ============================================================
// 工具函数
// ============================================================
bool cIsEqual(const Complex& a, const Complex& b, double epsilon = CALC_EPSILON);
Complex cNeg(const Complex& z);      // 取负
Complex cReciprocal(const Complex& z);  // 倒数 1/z

#endif // COMPLEX_NUMBER_H
