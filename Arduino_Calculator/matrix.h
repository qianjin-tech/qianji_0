/**
 * Matrix Library for Arduino Calculator
 * 支持矩阵的加减乘、转置、行列式、求逆等操作
 * 最大支持 MAX_MATRIX_SIZE x MAX_MATRIX_SIZE 矩阵
 */

#ifndef MATRIX_H
#define MATRIX_H

#include "config.h"
#include <math.h>

// ============================================================
// 矩阵结构体
// ============================================================
struct Matrix {
  uint8_t rows;           // 行数
  uint8_t cols;           // 列数
  double data[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE];

  // 构造函数
  Matrix(uint8_t r = 0, uint8_t c = 0) : rows(r), cols(c) {
    for (uint8_t i = 0; i < MAX_MATRIX_SIZE; i++) {
      for (uint8_t j = 0; j < MAX_MATRIX_SIZE; j++) {
        data[i][j] = 0.0;
      }
    }
  }

  // 设置为单位矩阵
  void setIdentity() {
    for (uint8_t i = 0; i < rows && i < cols; i++) {
      data[i][i] = 1.0;
    }
  }

  // 获取元素
  double get(uint8_t r, uint8_t c) const {
    if (r < rows && c < cols) return data[r][c];
    return 0.0;
  }

  // 设置元素
  void set(uint8_t r, uint8_t c, double val) {
    if (r < rows && c < cols) data[r][c] = val;
  }

  // 格式化输出到字符串（单行摘要）
  void toString(char* buffer, uint8_t len) const;

  // 打印到串口（调试用）
  void print() const;
};

// ============================================================
// 矩阵基本运算
// ============================================================
bool mAdd(const Matrix& a, const Matrix& b, Matrix& result);      // 矩阵加法
bool mSub(const Matrix& a, const Matrix& b, Matrix& result);      // 矩阵减法
bool mMul(const Matrix& a, const Matrix& b, Matrix& result);      // 矩阵乘法
bool mScalarMul(const Matrix& a, double scalar, Matrix& result);  // 数乘
bool mTranspose(const Matrix& a, Matrix& result);                  // 转置

// ============================================================
// 矩阵高级运算
// ============================================================
double mDeterminant(const Matrix& a);                             // 行列式
bool mInverse(const Matrix& a, Matrix& result);                   // 矩阵求逆
bool mAdjoint(const Matrix& a, Matrix& result);                   // 伴随矩阵
int mRank(const Matrix& a);                                       // 矩阵秩

// ============================================================
// 矩阵分解（简化版）
// ============================================================
bool mLUDecompose(const Matrix& a, Matrix& L, Matrix& U);         // LU分解

// ============================================================
// 线性方程组求解
// ============================================================
bool mSolveLinear(const Matrix& A, const Matrix& b, Matrix& x);   // Ax = b

// ============================================================
// 矩阵属性
// ============================================================
bool mIsSquare(const Matrix& a);                                  // 是否方阵
bool mIsSymmetric(const Matrix& a);                               // 是否对称
bool mIsDiagonal(const Matrix& a);                                // 是否对角
bool mIsIdentity(const Matrix& a);                                // 是否单位矩阵
bool mIsZero(const Matrix& a);                                    // 是否零矩阵

// ============================================================
// 工具函数
// ============================================================
bool mIsEqual(const Matrix& a, const Matrix& b, double epsilon = CALC_EPSILON);
void mCopy(const Matrix& src, Matrix& dest);                      // 矩阵复制

#endif // MATRIX_H
