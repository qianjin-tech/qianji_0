/**
 * Matrix Library Implementation
 */

#include "matrix.h"

void Matrix::toString(char* buffer, uint8_t len) const {
  snprintf(buffer, len, "[%dx%d]", rows, cols);
}

void Matrix::print() const {
  Serial.print("Matrix ");
  Serial.print(rows);
  Serial.print("x");
  Serial.println(cols);
  for (uint8_t i = 0; i < rows; i++) {
    for (uint8_t j = 0; j < cols; j++) {
      Serial.print(data[i][j], 4);
      Serial.print("\t");
    }
    Serial.println();
  }
}

bool mAdd(const Matrix& a, const Matrix& b, Matrix& result) {
  if (a.rows != b.rows || a.cols != b.cols) {
    return false;
  }
  result.rows = a.rows;
  result.cols = a.cols;
  for (uint8_t i = 0; i < a.rows; i++) {
    for (uint8_t j = 0; j < a.cols; j++) {
      result.data[i][j] = a.data[i][j] + b.data[i][j];
    }
  }
  return true;
}

bool mSub(const Matrix& a, const Matrix& b, Matrix& result) {
  if (a.rows != b.rows || a.cols != b.cols) {
    return false;
  }
  result.rows = a.rows;
  result.cols = a.cols;
  for (uint8_t i = 0; i < a.rows; i++) {
    for (uint8_t j = 0; j < a.cols; j++) {
      result.data[i][j] = a.data[i][j] - b.data[i][j];
    }
  }
  return true;
}

bool mMul(const Matrix& a, const Matrix& b, Matrix& result) {
  if (a.cols != b.rows) {
    return false;
  }
  result.rows = a.rows;
  result.cols = b.cols;
  for (uint8_t i = 0; i < result.rows; i++) {
    for (uint8_t j = 0; j < result.cols; j++) {
      result.data[i][j] = 0.0;
    }
  }
  for (uint8_t i = 0; i < a.rows; i++) {
    for (uint8_t j = 0; j < b.cols; j++) {
      for (uint8_t k = 0; k < a.cols; k++) {
        result.data[i][j] += a.data[i][k] * b.data[k][j];
      }
    }
  }
  return true;
}

bool mScalarMul(const Matrix& a, double scalar, Matrix& result) {
  result.rows = a.rows;
  result.cols = a.cols;
  for (uint8_t i = 0; i < a.rows; i++) {
    for (uint8_t j = 0; j < a.cols; j++) {
      result.data[i][j] = a.data[i][j] * scalar;
    }
  }
  return true;
}

bool mTranspose(const Matrix& a, Matrix& result) {
  result.rows = a.cols;
  result.cols = a.rows;
  for (uint8_t i = 0; i < a.rows; i++) {
    for (uint8_t j = 0; j < a.cols; j++) {
      result.data[j][i] = a.data[i][j];
    }
  }
  return true;
}

double mDeterminant(const Matrix& a) {
  if (!mIsSquare(a)) return NAN;
  uint8_t n = a.rows;

  if (n == 1) return a.data[0][0];
  if (n == 2) {
    return a.data[0][0] * a.data[1][1] - a.data[0][1] * a.data[1][0];
  }
  if (n == 3) {
    return a.data[0][0] * (a.data[1][1] * a.data[2][2] - a.data[1][2] * a.data[2][1])
         - a.data[0][1] * (a.data[1][0] * a.data[2][2] - a.data[1][2] * a.data[2][0])
         + a.data[0][2] * (a.data[1][0] * a.data[2][1] - a.data[1][1] * a.data[2][0]);
  }

  double det = 0.0;
  Matrix minorMat(0, 0);
  for (uint8_t j = 0; j < n; j++) {
    uint8_t mi = 0;
    for (uint8_t i = 1; i < n; i++) {
      uint8_t mj = 0;
      for (uint8_t k = 0; k < n; k++) {
        if (k == j) continue;
        minorMat.data[mi][mj] = a.data[i][k];
        mj++;
      }
      mi++;
    }
    minorMat.rows = n - 1;
    minorMat.cols = n - 1;
    double sign = (j % 2 == 0) ? 1.0 : -1.0;
    det += sign * a.data[0][j] * mDeterminant(minorMat);
  }
  return det;
}

bool mAdjoint(const Matrix& a, Matrix& result) {
  if (!mIsSquare(a)) return false;
  uint8_t n = a.rows;
  result.rows = n;
  result.cols = n;

  if (n == 1) {
    result.data[0][0] = 1.0;
    return true;
  }

  Matrix minorMat(0, 0);
  for (uint8_t i = 0; i < n; i++) {
    for (uint8_t j = 0; j < n; j++) {
      uint8_t mi = 0;
      for (uint8_t r = 0; r < n; r++) {
        if (r == i) continue;
        uint8_t mj = 0;
        for (uint8_t c = 0; c < n; c++) {
          if (c == j) continue;
          minorMat.data[mi][mj] = a.data[r][c];
          mj++;
        }
        mi++;
      }
      minorMat.rows = n - 1;
      minorMat.cols = n - 1;
      double sign = ((i + j) % 2 == 0) ? 1.0 : -1.0;
      result.data[j][i] = sign * mDeterminant(minorMat);
    }
  }
  return true;
}

bool mInverse(const Matrix& a, Matrix& result) {
  if (!mIsSquare(a)) return false;
  double det = mDeterminant(a);
  if (fabs(det) < CALC_EPSILON) return false;

  Matrix adj;
  if (!mAdjoint(a, adj)) return false;

  return mScalarMul(adj, 1.0 / det, result);
}

int mRank(const Matrix& a) {
  Matrix m = a;
  uint8_t rank = 0;
  uint8_t row = 0;

  for (uint8_t col = 0; col < m.cols && row < m.rows; col++) {
    uint8_t pivot = row;
    double maxVal = fabs(m.data[row][col]);
    for (uint8_t i = row + 1; i < m.rows; i++) {
      if (fabs(m.data[i][col]) > maxVal) {
        maxVal = fabs(m.data[i][col]);
        pivot = i;
      }
    }

    if (fabs(m.data[pivot][col]) < CALC_EPSILON) continue;

    if (pivot != row) {
      for (uint8_t j = 0; j < m.cols; j++) {
        double temp = m.data[row][j];
        m.data[row][j] = m.data[pivot][j];
        m.data[pivot][j] = temp;
      }
    }

    for (uint8_t i = row + 1; i < m.rows; i++) {
      double factor = m.data[i][col] / m.data[row][col];
      for (uint8_t j = col; j < m.cols; j++) {
        m.data[i][j] -= factor * m.data[row][j];
      }
    }

    rank++;
    row++;
  }

  return rank;
}

bool mLUDecompose(const Matrix& a, Matrix& L, Matrix& U) {
  if (!mIsSquare(a)) return false;
  uint8_t n = a.rows;

  L.rows = n; L.cols = n;
  U.rows = n; U.cols = n;

  for (uint8_t i = 0; i < n; i++) {
    for (uint8_t j = 0; j < n; j++) {
      L.data[i][j] = 0.0;
      U.data[i][j] = 0.0;
    }
  }

  for (uint8_t i = 0; i < n; i++) {
    for (uint8_t j = i; j < n; j++) {
      double sum = 0.0;
      for (uint8_t k = 0; k < i; k++) {
        sum += L.data[i][k] * U.data[k][j];
      }
      U.data[i][j] = a.data[i][j] - sum;
    }

    for (uint8_t j = i; j < n; j++) {
      if (i == j) {
        L.data[i][i] = 1.0;
      } else {
        double sum = 0.0;
        for (uint8_t k = 0; k < i; k++) {
          sum += L.data[j][k] * U.data[k][i];
        }
        if (fabs(U.data[i][i]) < CALC_EPSILON) return false;
        L.data[j][i] = (a.data[j][i] - sum) / U.data[i][i];
      }
    }
  }
  return true;
}

bool mSolveLinear(const Matrix& A, const Matrix& b, Matrix& x) {
  if (!mIsSquare(A) || b.cols != 1 || b.rows != A.rows) return false;

  Matrix L, U;
  if (!mLUDecompose(A, L, U)) return false;

  uint8_t n = A.rows;
  x.rows = n;
  x.cols = 1;

  double y[MAX_MATRIX_SIZE];
  for (uint8_t i = 0; i < n; i++) {
    double sum = 0.0;
    for (uint8_t j = 0; j < i; j++) {
      sum += L.data[i][j] * y[j];
    }
    y[i] = b.data[i][0] - sum;
  }

  for (int i = n - 1; i >= 0; i--) {
    double sum = 0.0;
    for (uint8_t j = i + 1; j < n; j++) {
      sum += U.data[i][j] * x.data[j][0];
    }
    if (fabs(U.data[i][i]) < CALC_EPSILON) return false;
    x.data[i][0] = (y[i] - sum) / U.data[i][i];
  }

  return true;
}

bool mIsSquare(const Matrix& a) {
  return a.rows == a.cols && a.rows > 0;
}

bool mIsSymmetric(const Matrix& a) {
  if (!mIsSquare(a)) return false;
  for (uint8_t i = 0; i < a.rows; i++) {
    for (uint8_t j = i + 1; j < a.cols; j++) {
      if (fabs(a.data[i][j] - a.data[j][i]) > CALC_EPSILON) return false;
    }
  }
  return true;
}

bool mIsDiagonal(const Matrix& a) {
  if (!mIsSquare(a)) return false;
  for (uint8_t i = 0; i < a.rows; i++) {
    for (uint8_t j = 0; j < a.cols; j++) {
      if (i != j && fabs(a.data[i][j]) > CALC_EPSILON) return false;
    }
  }
  return true;
}

bool mIsIdentity(const Matrix& a) {
  if (!mIsSquare(a)) return false;
  for (uint8_t i = 0; i < a.rows; i++) {
    for (uint8_t j = 0; j < a.cols; j++) {
      if (i == j) {
        if (fabs(a.data[i][j] - 1.0) > CALC_EPSILON) return false;
      } else {
        if (fabs(a.data[i][j]) > CALC_EPSILON) return false;
      }
    }
  }
  return true;
}

bool mIsZero(const Matrix& a) {
  for (uint8_t i = 0; i < a.rows; i++) {
    for (uint8_t j = 0; j < a.cols; j++) {
      if (fabs(a.data[i][j]) > CALC_EPSILON) return false;
    }
  }
  return true;
}

bool mIsEqual(const Matrix& a, const Matrix& b, double epsilon) {
  if (a.rows != b.rows || a.cols != b.cols) return false;
  for (uint8_t i = 0; i < a.rows; i++) {
    for (uint8_t j = 0; j < a.cols; j++) {
      if (fabs(a.data[i][j] - b.data[i][j]) > epsilon) return false;
    }
  }
  return true;
}

void mCopy(const Matrix& src, Matrix& dest) {
  dest.rows = src.rows;
  dest.cols = src.cols;
  for (uint8_t i = 0; i < MAX_MATRIX_SIZE; i++) {
    for (uint8_t j = 0; j < MAX_MATRIX_SIZE; j++) {
      dest.data[i][j] = src.data[i][j];
    }
  }
}
