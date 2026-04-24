/**
 * Arduino Advanced Calculator - Configuration File
 * Hardware: Arduino Uno/Nano (ATmega328P)
 * Author: qianjin-tech
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <avr/pgmspace.h>

// ============================================================
// 版本信息
// ============================================================
#define CALC_VERSION "v1.0"
#define CALC_NAME    "Arduino Calc"

// ============================================================
// 硬件引脚配置 - LCD2004 (I2C)
// ============================================================
// LCD2004 I2C 地址，常见为 0x27 或 0x3F
#define LCD_I2C_ADDR    0x27
#define LCD_COLS        20
#define LCD_ROWS        4

// ============================================================
// 硬件引脚配置 - 5x5 矩阵键盘
// ============================================================
// 行引脚（输出）
#define KEY_ROW0        2
#define KEY_ROW1        3
#define KEY_ROW2        4
#define KEY_ROW3        5
#define KEY_ROW4        6

// 列引脚（输入，带上拉电阻）
#define KEY_COL0        7
#define KEY_COL1        8
#define KEY_COL2        9
#define KEY_COL3        10
#define KEY_COL4        11

// ============================================================
// 键盘布局定义（5行 x 5列）
// ============================================================
// 模式切换键
#define KEY_MODE        24   // 最后一个键作为模式切换

// 基础模式按键定义
const char KEY_MAP_NORMAL[5][5] PROGMEM = {
  {'7', '8', '9', '/', 'C'},   // C = Clear
  {'4', '5', '6', '*', 'D'},   // D = Delete
  {'1', '2', '3', '-', 'S'},   // S = Shift/2nd
  {'0', '.', '=', '+', 'A'},   // A = Alpha
  {'(', ')', '^', 'M', 'E'}    // M = Mode切换, E = Enter/确认
};

// 2nd模式按键定义（科学函数）
const char KEY_MAP_2ND[5][5] PROGMEM = {
  {'s', 'c', 't', 'l', 'C'},   // s=sin, c=cos, t=tan, l=log
  {'S', 'C', 'T', 'L', 'D'},   // S=asin, C=acos, T=atan, L=ln
  {'e', 'p', 'r', 'q', 'S'},   // e=exp, p=pow, r=sqrt, q=cbrt
  {'P', 'E', '!', 'a', 'A'},   // P=PI, E=e, !=factorial, a=abs
  {'h', 'H', 'R', 'M', 'E'}    // h=sinh, H=cosh, R=tanh
};

// ALPHA模式按键定义（变量与矩阵）
const char KEY_MAP_ALPHA[5][5] PROGMEM = {
  {'A', 'B', 'C', 'i', 'C'},   // A,B,C=变量, i=虚数单位
  {'D', 'E', 'F', 'G', 'D'},
  {'m', 'n', 'o', 'p', 'S'},   // m=matrix模式
  {'x', 'y', 'z', 'w', 'A'},
  {'[', ']', ';', 'M', 'E'}    // []=矩阵括号, ; = 分隔符
};

// ============================================================
// 计算参数配置
// ============================================================
#define MAX_EXPR_LENGTH     80      // 最大表达式长度
#define MAX_STACK_SIZE      40      // 运算栈最大深度
#define MAX_MATRIX_SIZE     4       // 矩阵最大维度（4x4）
#define MAX_VARIABLES       10      // 变量存储数量 A-J
#define DISPLAY_PRECISION   6       // 显示小数位数

// ============================================================
// 数学常量
// ============================================================
#define CALC_PI             3.14159265358979323846
#define CALC_E              2.71828182845904523536
#define CALC_EPSILON        1e-10    // 浮点比较精度

// ============================================================
// 角度模式
// ============================================================
enum AngleMode {
  DEGREE = 0,   // 角度制
  RADIAN = 1    // 弧度制
};

// ============================================================
// 计算器模式
// ============================================================
enum CalcMode {
  MODE_NORMAL = 0,    // 普通计算模式
  MODE_2ND    = 1,    // 第二功能模式
  MODE_ALPHA  = 2,    // 字母/变量模式
  MODE_MATRIX = 3,    // 矩阵模式
  MODE_COMPLEX= 4     // 复数模式
};

// ============================================================
// 错误代码
// ============================================================
enum ErrorCode {
  ERR_NONE        = 0,
  ERR_SYNTAX      = 1,    // 语法错误
  ERR_DIV_ZERO    = 2,    // 除零错误
  ERR_OVERFLOW    = 3,    // 溢出
  ERR_DOMAIN      = 4,    // 定义域错误（如负数开方）
  ERR_MEMORY      = 5,    // 内存不足
  ERR_MATRIX_DIM  = 6,    // 矩阵维度不匹配
  ERR_SINGULAR    = 7,    // 矩阵奇异（不可逆）
  ERR_COMPLEX     = 8,    // 复数运算错误
  ERR_UNKNOWN     = 99    // 未知错误
};

// ============================================================
// 错误信息字符串（存储在 Flash 中节省 SRAM）
// ============================================================
const char ERR_MSG_NONE[]       PROGMEM = "OK";
const char ERR_MSG_SYNTAX[]     PROGMEM = "Syntax Error";
const char ERR_MSG_DIV_ZERO[]   PROGMEM = "Div by Zero";
const char ERR_MSG_OVERFLOW[]   PROGMEM = "Overflow";
const char ERR_MSG_DOMAIN[]     PROGMEM = "Domain Error";
const char ERR_MSG_MEMORY[]     PROGMEM = "Memory Full";
const char ERR_MSG_MATRIX_DIM[] PROGMEM = "Matrix Dim Err";
const char ERR_MSG_SINGULAR[]   PROGMEM = "Singular Mat";
const char ERR_MSG_COMPLEX[]    PROGMEM = "Complex Err";
const char ERR_MSG_UNKNOWN[]    PROGMEM = "Error";

// 错误信息表
const char* const ERROR_MESSAGES[] PROGMEM = {
  ERR_MSG_NONE,
  ERR_MSG_SYNTAX,
  ERR_MSG_DIV_ZERO,
  ERR_MSG_OVERFLOW,
  ERR_MSG_DOMAIN,
  ERR_MSG_MEMORY,
  ERR_MSG_MATRIX_DIM,
  ERR_MSG_SINGULAR,
  ERR_MSG_COMPLEX,
  ERR_MSG_UNKNOWN
};

// ============================================================
// 辅助宏
// ============================================================
#define IS_DIGIT(c)         ((c) >= '0' && (c) <= '9')
#define IS_ALPHA(c)         (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define IS_OPERATOR(c)      ((c) == '+' || (c) == '-' || (c) == '*' || (c) == '/' || (c) == '^')
#define DEG_TO_RAD(d)       ((d) * CALC_PI / 180.0)
#define RAD_TO_DEG(r)       ((r) * 180.0 / CALC_PI)

// 获取错误信息字符串（从 PROGMEM 读取到临时缓冲区）
inline void getErrorMessage(ErrorCode err, char* buffer, uint8_t len) {
  if (err >= 0 && err <= 9) {
    strncpy_P(buffer, (const char*)pgm_read_word(&ERROR_MESSAGES[err]), len - 1);
    buffer[len - 1] = '\0';
  } else {
    strncpy_P(buffer, ERR_MSG_UNKNOWN, len - 1);
    buffer[len - 1] = '\0';
  }
}

#endif // CONFIG_H
