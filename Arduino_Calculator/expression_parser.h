/**
 * Expression Parser for Arduino Calculator
 * 支持中缀表达式解析，处理运算符优先级、括号、函数调用
 * 使用调度场算法（Shunting Yard）将中缀表达式转为后缀表达式，再求值
 */

#ifndef EXPRESSION_PARSER_H
#define EXPRESSION_PARSER_H

#include "config.h"
#include "scientific_math.h"
#include "complex_number.h"
#include "matrix.h"

// ============================================================
// 表达式求值结果类型
// ============================================================
enum ValueType {
  VAL_NONE = 0,
  VAL_NUMBER = 1,    // 实数
  VAL_COMPLEX = 2,   // 复数
  VAL_MATRIX = 3     // 矩阵
};

struct Value {
  ValueType type;
  double number;       // 实数值
  Complex complex;     // 复数值
  Matrix matrix;       // 矩阵值

  Value() : type(VAL_NONE), number(0.0) {}
};

// ============================================================
// 解析器状态
// ============================================================
struct ParserState {
  char expr[MAX_EXPR_LENGTH];     // 当前表达式
  uint8_t pos;                     // 当前解析位置
  ErrorCode error;                 // 错误状态
  CalcMode mode;                   // 当前计算模式
  ValueType expectedType;          // 期望的结果类型

  ParserState() : pos(0), error(ERR_NONE), mode(MODE_NORMAL), expectedType(VAL_NUMBER) {
    expr[0] = '\0';
  }
};

// ============================================================
// 核心解析函数
// ============================================================

// 解析并求值表达式，返回结果
Value evaluateExpression(const char* expression, CalcMode mode = MODE_NORMAL);

// 获取最后一次解析的错误
ErrorCode getLastError();

// 获取错误信息字符串
void getLastErrorMessage(char* buffer, uint8_t len);

// ============================================================
// 变量存储（A-J，共10个变量）
// ============================================================
bool storeVariable(char name, const Value& value);
bool recallVariable(char name, Value& value);
void clearVariables();

// ============================================================
// 矩阵存储（M1-M5，共5个矩阵）
// ============================================================
bool storeMatrix(uint8_t index, const Matrix& mat);
bool recallMatrix(uint8_t index, Matrix& mat);
void clearMatrices();

#endif // EXPRESSION_PARSER_H
