/**
 * Complex Number Library Implementation
 */

#include "complex_number.h"
#include "scientific_math.h"

// ============================================================
// 格式化输出
// ============================================================
void Complex::toString(char* buffer, uint8_t len, uint8_t precision) const {
  char fmt[16];
  snprintf(fmt, sizeof(fmt), "%%.%df", precision);

  if (isZero()) {
    strncpy(buffer, "0", len);
    return;
  }

  if (isReal()) {
    snprintf(buffer, len, fmt, real);
    return;
  }

  if (isImaginary()) {
    if (fabs(imag - 1.0) < CALC_EPSILON) {
      strncpy(buffer, "i", len);
    } else if (fabs(imag + 1.0) < CALC_EPSILON) {
      strncpy(buffer, "-i", len);
    } else {
      char temp[32];
      snprintf(temp, sizeof(temp), fmt, imag);
      snprintf(buffer, len, "%si", temp);
    }
    return;
  }

  // 一般复数 a + bi
  char realStr[32], imagStr[32];
  snprintf(realStr, sizeof(realStr), fmt, real);

  if (fabs(imag - 1.0) < CALC_EPSILON) {
    snprintf(imagStr, sizeof(imagStr), "i");
  } else if (fabs(imag + 1.0) < CALC_EPSILON) {
    snprintf(imagStr, sizeof(imagStr), "-i");
  } else {
    char temp[32];
    snprintf(temp, sizeof(temp), fmt, fabs(imag));
    snprintf(imagStr, sizeof(imagStr), "%si", temp);
  }

  if (imag > 0) {
    snprintf(buffer, len, "%s+%s", realStr, imagStr);
  } else {
    snprintf(buffer, len, "%s-%s", realStr, imagStr);
  }
}

// ============================================================
// 基本运算
// ============================================================
Complex cAdd(const Complex& a, const Complex& b) {
  return Complex(a.real + b.real, a.imag + b.imag);
}

Complex cSub(const Complex& a, const Complex& b) {
  return Complex(a.real - b.real, a.imag - b.imag);
}

Complex cMul(const Complex& a, const Complex& b) {
  return Complex(
    a.real * b.real - a.imag * b.imag,
    a.real * b.imag + a.imag * b.real
  );
}

Complex cDiv(const Complex& a, const Complex& b) {
  if (b.isZero()) {
    return Complex(NAN, NAN);
  }
  double denom = b.real * b.real + b.imag * b.imag;
  return Complex(
    (a.real * b.real + a.imag * b.imag) / denom,
    (a.imag * b.real - a.real * b.imag) / denom
  );
}

// ============================================================
// 复数属性
// ============================================================
double cAbs(const Complex& z) {
  return sqrt(z.real * z.real + z.imag * z.imag);
}

double cArg(const Complex& z) {
  if (z.isZero()) return NAN;
  return atan2(z.imag, z.real);
}

Complex cConj(const Complex& z) {
  return Complex(z.real, -z.imag);
}

// ============================================================
// 复数科学函数
// ============================================================
Complex cExp(const Complex& z) {
  double e_real = exp(z.real);
  return Complex(
    e_real * cos(z.imag),
    e_real * sin(z.imag)
  );
}

Complex cLn(const Complex& z) {
  if (z.isZero()) return Complex(NAN, NAN);
  double r = cAbs(z);
  double theta = cArg(z);
  return Complex(log(r), theta);
}

Complex cPow(const Complex& base, const Complex& exp) {
  if (base.isZero()) {
    if (exp.isZero()) return Complex(NAN, NAN);
    if (exp.real > 0) return Complex(0.0, 0.0);
    return Complex(NAN, NAN);
  }

  // z1^z2 = exp(z2 * ln(z1))
  Complex ln_base = cLn(base);
  Complex product = cMul(exp, ln_base);
  return cExp(product);
}

Complex cSqrt(const Complex& z) {
  if (z.isZero()) return Complex(0.0, 0.0);
  double r = cAbs(z);
  double theta = cArg(z);
  double sqrtR = sqrt(r);
  return Complex(
    sqrtR * cos(theta / 2.0),
    sqrtR * sin(theta / 2.0)
  );
}

// ============================================================
// 复数三角函数
// ============================================================
Complex cSin(const Complex& z) {
  // sin(a+bi) = sin(a)cosh(b) + i*cos(a)sinh(b)
  return Complex(
    sin(z.real) * cosh(z.imag),
    cos(z.real) * sinh(z.imag)
  );
}

Complex cCos(const Complex& z) {
  // cos(a+bi) = cos(a)cosh(b) - i*sin(a)sinh(b)
  return Complex(
    cos(z.real) * cosh(z.imag),
    -sin(z.real) * sinh(z.imag)
  );
}

Complex cTan(const Complex& z) {
  Complex s = cSin(z);
  Complex c = cCos(z);
  return cDiv(s, c);
}

// ============================================================
// 极坐标/直角坐标转换
// ============================================================
Complex polarToRect(double r, double theta) {
  return Complex(
    r * cos(theta),
    r * sin(theta)
  );
}

void rectToPolar(const Complex& z, double& r, double& theta) {
  r = cAbs(z);
  theta = cArg(z);
}

// ============================================================
// 工具函数
// ============================================================
bool cIsEqual(const Complex& a, const Complex& b, double epsilon) {
  return fabs(a.real - b.real) < epsilon && fabs(a.imag - b.imag) < epsilon;
}

Complex cNeg(const Complex& z) {
  return Complex(-z.real, -z.imag);
}

Complex cReciprocal(const Complex& z) {
  return cDiv(Complex(1.0, 0.0), z);
}
