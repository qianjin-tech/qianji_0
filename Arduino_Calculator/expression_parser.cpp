/**
 * Expression Parser Implementation
 * Shunting Yard Algorithm + Recursive Descent for evaluation
 */

#include "expression_parser.h"

// ============================================================
// 全局状态
// ============================================================
static ParserState g_parser;
static Value g_variables[10];     // A-J
static Matrix g_matrices[5];      // M1-M5

// ============================================================
// 辅助函数
// ============================================================
static void skipSpaces() {
  while (g_parser.expr[g_parser.pos] == ' ') {
    g_parser.pos++;
  }
}

static bool isAtEnd() {
  return g_parser.expr[g_parser.pos] == '\0';
}

static char peek() {
  return g_parser.expr[g_parser.pos];
}

static char advance() {
  return g_parser.expr[g_parser.pos++];
}

static bool match(char expected) {
  if (peek() == expected) {
    g_parser.pos++;
    return true;
  }
  return false;
}

// ============================================================
// 运算符优先级
// ============================================================
static int getPrecedence(char op) {
  switch (op) {
    case '+':
    case '-': return 1;
    case '*':
    case '/': return 2;
    case '^': return 3;
    case '~': return 4;  // 一元负号
    default: return 0;
  }
}

static bool isRightAssociative(char op) {
  return op == '^';
}

// ============================================================
// 解析数字（支持小数和科学计数法）
// ============================================================
static double parseNumber() {
  skipSpaces();
  uint8_t start = g_parser.pos;
  bool hasDot = false;

  // 处理负号（如果前面是运算符或括号开头）
  if (peek() == '-') {
    // 一元负号在更高层处理
  }

  while (IS_DIGIT(peek()) || peek() == '.') {
    if (peek() == '.') {
      if (hasDot) break;  // 第二个小数点
      hasDot = true;
    }
    advance();
  }

  // 科学计数法 e.g. 1.5e-3
  if (peek() == 'e' || peek() == 'E') {
    uint8_t savePos = g_parser.pos;
    advance();
    if (peek() == '+' || peek() == '-') {
      advance();
    }
    if (!IS_DIGIT(peek())) {
      g_parser.pos = savePos;  // 回退
    } else {
      while (IS_DIGIT(peek())) advance();
    }
  }

  char numStr[32];
  uint8_t len = g_parser.pos - start;
  if (len >= sizeof(numStr)) len = sizeof(numStr) - 1;
  strncpy(numStr, g_parser.expr + start, len);
  numStr[len] = '\0';

  return atof(numStr);
}

// ============================================================
// 解析函数名
// ============================================================
static bool parseFunctionName(char* name, uint8_t maxLen) {
  skipSpaces();
  uint8_t start = g_parser.pos;
  while (IS_ALPHA(peek()) && (g_parser.pos - start) < maxLen - 1) {
    advance();
  }
  uint8_t len = g_parser.pos - start;
  if (len == 0) return false;
  strncpy(name, g_parser.expr + start, len);
  name[len] = '\0';
  return true;
}

// ============================================================
// 前向声明
// ============================================================
static Value parseExpression();
static Value parseTerm();
static Value parseFactor();
static Value parsePrimary();

// ============================================================
// 表达式解析：处理加减法
// ============================================================
static Value parseExpression() {
  Value left = parseTerm();
  if (g_parser.error != ERR_NONE) return left;

  while (true) {
    skipSpaces();
    char op = peek();
    if (op != '+' && op != '-') break;
    advance();

    Value right = parseTerm();
    if (g_parser.error != ERR_NONE) return left;

    // 类型检查与运算
    if (left.type == VAL_NUMBER && right.type == VAL_NUMBER) {
      if (op == '+') left.number += right.number;
      else left.number -= right.number;
    }
    else if (left.type == VAL_COMPLEX && right.type == VAL_COMPLEX) {
      if (op == '+') left.complex = cAdd(left.complex, right.complex);
      else left.complex = cSub(left.complex, right.complex);
    }
    else if (left.type == VAL_MATRIX && right.type == VAL_MATRIX) {
      Matrix result;
      bool ok = (op == '+') ? mAdd(left.matrix, right.matrix, result)
                            : mSub(left.matrix, right.matrix, result);
      if (!ok) {
        g_parser.error = ERR_MATRIX_DIM;
        return left;
      }
      left.matrix = result;
    }
    else {
      g_parser.error = ERR_SYNTAX;
      return left;
    }
  }

  return left;
}

// ============================================================
// 项解析：处理乘除法
// ============================================================
static Value parseTerm() {
  Value left = parseFactor();
  if (g_parser.error != ERR_NONE) return left;

  while (true) {
    skipSpaces();
    char op = peek();
    if (op != '*' && op != '/') break;
    advance();

    Value right = parseFactor();
    if (g_parser.error != ERR_NONE) return left;

    if (left.type == VAL_NUMBER && right.type == VAL_NUMBER) {
      if (op == '*') left.number *= right.number;
      else {
        if (fabs(right.number) < CALC_EPSILON) {
          g_parser.error = ERR_DIV_ZERO;
          return left;
        }
        left.number /= right.number;
      }
    }
    else if (left.type == VAL_COMPLEX && right.type == VAL_COMPLEX) {
      if (op == '*') left.complex = cMul(left.complex, right.complex);
      else left.complex = cDiv(left.complex, right.complex);
    }
    else if (left.type == VAL_MATRIX && right.type == VAL_MATRIX) {
      if (op == '*') {
        Matrix result;
        if (!mMul(left.matrix, right.matrix, result)) {
          g_parser.error = ERR_MATRIX_DIM;
          return left;
        }
        left.matrix = result;
      } else {
        g_parser.error = ERR_SYNTAX;  // 矩阵除法未定义
        return left;
      }
    }
    else if (left.type == VAL_MATRIX && right.type == VAL_NUMBER) {
      Matrix result;
      mScalarMul(left.matrix, (op == '*') ? right.number : (1.0 / right.number), result);
      left.matrix = result;
    }
    else {
      g_parser.error = ERR_SYNTAX;
      return left;
    }
  }

  return left;
}

// ============================================================
// 因子解析：处理幂运算
// ============================================================
static Value parseFactor() {
  Value left = parsePrimary();
  if (g_parser.error != ERR_NONE) return left;

  skipSpaces();
  if (match('^')) {
    Value right = parseFactor();  // 右结合
    if (g_parser.error != ERR_NONE) return left;

    if (left.type == VAL_NUMBER && right.type == VAL_NUMBER) {
      left.number = calc_pow(left.number, right.number);
      if (isnan(left.number)) g_parser.error = ERR_DOMAIN;
    }
    else if (left.type == VAL_COMPLEX && right.type == VAL_COMPLEX) {
      left.complex = cPow(left.complex, right.complex);
    }
    else {
      g_parser.error = ERR_SYNTAX;
    }
  }

  return left;
}

// ============================================================
// 主元解析：处理数字、变量、函数、括号
// ============================================================
static Value parsePrimary() {
  skipSpaces();
  Value result;

  // 处理一元负号
  if (match('-')) {
    Value val = parsePrimary();
    if (g_parser.error != ERR_NONE) return val;

    if (val.type == VAL_NUMBER) {
      val.number = -val.number;
    } else if (val.type == VAL_COMPLEX) {
      val.complex = cNeg(val.complex);
    } else if (val.type == VAL_MATRIX) {
      Matrix neg;
      mScalarMul(val.matrix, -1.0, neg);
      val.matrix = neg;
    }
    return val;
  }

  // 括号表达式
  if (match('(')) {
    result = parseExpression();
    skipSpaces();
    if (!match(')')) {
      g_parser.error = ERR_SYNTAX;
      return result;
    }
    return result;
  }

  // 复数括号 (a,b)
  if (match('[')) {
    // 矩阵解析
    result.type = VAL_MATRIX;
    result.matrix.rows = 0;
    result.matrix.cols = 0;
    uint8_t colCount = 0;
    uint8_t rowCount = 0;

    while (!isAtEnd() && peek() != ']') {
      skipSpaces();
      Value elem = parseExpression();
      if (g_parser.error != ERR_NONE) return result;

      if (elem.type == VAL_NUMBER) {
        result.matrix.data[rowCount][colCount] = elem.number;
        colCount++;
      }

      skipSpaces();
      if (match(',')) {
        continue;
      } else if (match(';')) {
        if (rowCount == 0) result.matrix.cols = colCount;
        rowCount++;
        colCount = 0;
      } else if (peek() == ']') {
        break;
      } else {
        g_parser.error = ERR_SYNTAX;
        return result;
      }
    }

    if (rowCount == 0) result.matrix.cols = colCount;
    result.matrix.rows = rowCount + 1;

    if (!match(']')) {
      g_parser.error = ERR_SYNTAX;
    }
    return result;
  }

  // 数字
  if (IS_DIGIT(peek()) || peek() == '.') {
    result.type = VAL_NUMBER;
    result.number = parseNumber();
    return result;
  }

  // 函数或变量
  if (IS_ALPHA(peek())) {
    char name[16];
    if (!parseFunctionName(name, sizeof(name))) {
      g_parser.error = ERR_SYNTAX;
      return result;
    }

    // 检查是否是变量
    if (strlen(name) == 1 && name[0] >= 'A' && name[0] <= 'J') {
      if (recallVariable(name[0], result)) {
        return result;
      }
    }

    // 检查是否是虚数单位
    if (strcmp(name, "i") == 0) {
      result.type = VAL_COMPLEX;
      result.complex = Complex(0.0, 1.0);
      return result;
    }

    // 检查是否是常数
    if (strcmp(name, "PI") == 0 || strcmp(name, "pi") == 0) {
      result.type = VAL_NUMBER;
      result.number = CALC_PI;
      return result;
    }
    if (strcmp(name, "E") == 0 || strcmp(name, "e") == 0) {
      result.type = VAL_NUMBER;
      result.number = CALC_E;
      return result;
    }

    // 函数调用
    skipSpaces();
    if (!match('(')) {
      g_parser.error = ERR_SYNTAX;
      return result;
    }

    Value arg = parseExpression();
    if (g_parser.error != ERR_NONE) return result;

    skipSpaces();
    if (!match(')')) {
      g_parser.error = ERR_SYNTAX;
      return result;
    }

    // 执行函数
    result.type = VAL_NUMBER;
    if (strcmp(name, "sin") == 0) {
      if (arg.type == VAL_NUMBER) result.number = calc_sin(arg.number);
      else if (arg.type == VAL_COMPLEX) { result.type = VAL_COMPLEX; result.complex = cSin(arg.complex); }
    }
    else if (strcmp(name, "cos") == 0) {
      if (arg.type == VAL_NUMBER) result.number = calc_cos(arg.number);
      else if (arg.type == VAL_COMPLEX) { result.type = VAL_COMPLEX; result.complex = cCos(arg.complex); }
    }
    else if (strcmp(name, "tan") == 0) {
      if (arg.type == VAL_NUMBER) result.number = calc_tan(arg.number);
      else if (arg.type == VAL_COMPLEX) { result.type = VAL_COMPLEX; result.complex = cTan(arg.complex); }
    }
    else if (strcmp(name, "asin") == 0) result.number = calc_asin(arg.number);
    else if (strcmp(name, "acos") == 0) result.number = calc_acos(arg.number);
    else if (strcmp(name, "atan") == 0) result.number = calc_atan(arg.number);
    else if (strcmp(name, "sinh") == 0) result.number = calc_sinh(arg.number);
    else if (strcmp(name, "cosh") == 0) result.number = calc_cosh(arg.number);
    else if (strcmp(name, "tanh") == 0) result.number = calc_tanh(arg.number);
    else if (strcmp(name, "ln") == 0) result.number = calc_ln(arg.number);
    else if (strcmp(name, "log") == 0) result.number = calc_log(arg.number);
    else if (strcmp(name, "exp") == 0) result.number = calc_exp(arg.number);
    else if (strcmp(name, "sqrt") == 0) result.number = calc_sqrt(arg.number);
    else if (strcmp(name, "cbrt") == 0) result.number = calc_cbrt(arg.number);
    else if (strcmp(name, "abs") == 0) result.number = calc_abs(arg.number);
    else if (strcmp(name, "floor") == 0) result.number = calc_floor(arg.number);
    else if (strcmp(name, "ceil") == 0) result.number = calc_ceil(arg.number);
    else if (strcmp(name, "round") == 0) result.number = calc_round(arg.number);
    else if (strcmp(name, "fact") == 0) result.number = calc_factorial(arg.number);
    else if (strcmp(name, "deg") == 0) result.number = RAD_TO_DEG(arg.number);
    else if (strcmp(name, "rad") == 0) result.number = DEG_TO_RAD(arg.number);
    else {
      g_parser.error = ERR_SYNTAX;
    }

    if (isnan(result.number) && result.type == VAL_NUMBER) {
      g_parser.error = ERR_DOMAIN;
    }

    return result;
  }

  g_parser.error = ERR_SYNTAX;
  return result;
}

// ============================================================
// 公共接口
// ============================================================
Value evaluateExpression(const char* expression, CalcMode mode) {
  // 重置状态
  g_parser.pos = 0;
  g_parser.error = ERR_NONE;
  g_parser.mode = mode;
  strncpy(g_parser.expr, expression, MAX_EXPR_LENGTH - 1);
  g_parser.expr[MAX_EXPR_LENGTH - 1] = '\0';

  Value result = parseExpression();
  skipSpaces();

  if (!isAtEnd() && g_parser.error == ERR_NONE) {
    g_parser.error = ERR_SYNTAX;
  }

  return result;
}

ErrorCode getLastError() {
  return g_parser.error;
}

void getLastErrorMessage(char* buffer, uint8_t len) {
  getErrorMessage(g_parser.error, buffer, len);
}

// ============================================================
// 变量存储
// ============================================================
bool storeVariable(char name, const Value& value) {
  if (name < 'A' || name > 'J') return false;
  g_variables[name - 'A'] = value;
  return true;
}

bool recallVariable(char name, Value& value) {
  if (name < 'A' || name > 'J') return false;
  value = g_variables[name - 'A'];
  return value.type != VAL_NONE;
}

void clearVariables() {
  for (uint8_t i = 0; i < 10; i++) {
    g_variables[i].type = VAL_NONE;
  }
}

// ============================================================
// 矩阵存储
// ============================================================
bool storeMatrix(uint8_t index, const Matrix& mat) {
  if (index < 1 || index > 5) return false;
  g_matrices[index - 1] = mat;
  return true;
}

bool recallMatrix(uint8_t index, Matrix& mat) {
  if (index < 1 || index > 5) return false;
  mat = g_matrices[index - 1];
  return mat.rows > 0;
}

void clearMatrices() {
  for (uint8_t i = 0; i < 5; i++) {
    g_matrices[i].rows = 0;
    g_matrices[i].cols = 0;
  }
}
