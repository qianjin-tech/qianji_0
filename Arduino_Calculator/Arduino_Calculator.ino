/**
 * Arduino Advanced Calculator
 * ============================================
 * A powerful scientific calculator for Arduino Uno/Nano
 * Features:
 *   - Basic arithmetic (+, -, *, /, ^)
 *   - Scientific functions (sin, cos, tan, log, ln, exp, etc.)
 *   - Complex number operations
 *   - Matrix operations (+, -, *, transpose, determinant, inverse)
 *   - Variable storage (A-J)
 *   - 5x5 matrix keyboard with mode switching
 *   - LCD2004 I2C display
 *
 * Hardware:
 *   - Arduino Uno/Nano
 *   - LCD2004 (I2C, address 0x27)
 *   - 5x5 matrix keypad
 *
 * Author: qianjin-tech
 * Version: v1.0
 */

#include "config.h"
#include "scientific_math.h"
#include "complex_number.h"
#include "matrix.h"
#include "expression_parser.h"
#include "keyboard.h"
#include "display.h"

// ============================================================
// 全局状态
// ============================================================
static char inputBuffer[MAX_EXPR_LENGTH];
static uint8_t inputPos = 0;
static CalcMode currentMode = MODE_NORMAL;
static AngleMode currentAngleMode = DEGREE;
static bool shiftActive = false;
static bool alphaActive = false;

// ============================================================
// 函数前向声明
// ============================================================
void handleKey(char key);
void processInput();
void clearInput();
void deleteLastChar();
void appendToInput(char c);
void appendStringToInput(const char* str);
void toggleMode();
void toggleAngleMode();
void showHelp();

// ============================================================
// 初始化
// ============================================================
void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }  // 等待串口连接（Leonardo/Micro需要）

  Serial.println(F("================================"));
  Serial.println(F("  Arduino Advanced Calculator"));
  Serial.print(F("  Version: "));
  Serial.println(F(CALC_VERSION));
  Serial.println(F("================================"));
  Serial.println();
  Serial.println(F("Initializing..."));

  // 初始化硬件
  keyboardInit();
  displayInit();

  // 初始化变量
  clearInput();
  clearVariables();
  clearMatrices();

  // 显示就绪信息
  displayStatus(currentMode, currentAngleMode);
  displayMessage("Ready");

  Serial.println(F("Calculator ready!"));
  Serial.println(F("Serial commands: 'c'=clear, 'a'=toggle angle, 'h'=help"));
}

// ============================================================
// 主循环
// ============================================================
void loop() {
  // 处理串口输入（调试用）
  if (Serial.available()) {
    char cmd = Serial.read();
    handleSerialCommand(cmd);
  }

  // 处理键盘输入
  char key;
  KeyEvent evt = getKeyEvent(key);

  if (evt == KEY_PRESSED) {
    handleKey(key);
  }

  delay(10);  // 小延迟降低CPU占用
}

// ============================================================
// 串口命令处理（调试）
// ============================================================
void handleSerialCommand(char cmd) {
  switch (cmd) {
    case 'c':
    case 'C':
      clearInput();
      displayClear();
      displayStatus(currentMode, currentAngleMode);
      displayMessage("Cleared");
      Serial.println(F("Input cleared"));
      break;

    case 'a':
    case 'A':
      toggleAngleMode();
      break;

    case 'h':
    case 'H':
    case '?':
      showHelp();
      break;

    case '\r':
    case '\n':
      processInput();
      break;

    case '\b':
    case 127:
      deleteLastChar();
      break;

    default:
      if (cmd >= 32 && cmd < 127) {
        appendToInput(cmd);
      }
      break;
  }
}

// ============================================================
// 按键处理
// ============================================================
void handleKey(char key) {
  // 模式切换键
  if (key == KEY_CHAR_SHIFT) {
    if (currentMode == MODE_2ND) {
      currentMode = MODE_NORMAL;
      shiftActive = false;
    } else {
      currentMode = MODE_2ND;
      shiftActive = true;
    }
    displayStatus(currentMode, currentAngleMode);
    return;
  }

  if (key == KEY_CHAR_ALPHA) {
    if (currentMode == MODE_ALPHA) {
      currentMode = MODE_NORMAL;
      alphaActive = false;
    } else {
      currentMode = MODE_ALPHA;
      alphaActive = true;
    }
    displayStatus(currentMode, currentAngleMode);
    return;
  }

  if (key == KEY_CHAR_MODE) {
    toggleMode();
    return;
  }

  // 根据当前模式转换按键
  char actualKey = getKeyForMode(key, currentMode);

  // 处理特殊功能键
  switch (actualKey) {
    case KEY_CHAR_CLEAR:
      clearInput();
      displayClear();
      displayStatus(currentMode, currentAngleMode);
      displayMessage("Cleared");
      return;

    case KEY_CHAR_DELETE:
      deleteLastChar();
      return;

    case KEY_CHAR_ENTER:
    case '=':
      processInput();
      return;
  }

  // 将按键字符添加到输入
  appendToInput(actualKey);

  // 如果按了非模式键，自动退出2nd/alpha模式
  if (currentMode == MODE_2ND && actualKey != KEY_CHAR_SHIFT) {
    currentMode = MODE_NORMAL;
    shiftActive = false;
    displayStatus(currentMode, currentAngleMode);
  }
}

// ============================================================
// 处理输入并计算
// ============================================================
void processInput() {
  if (inputPos == 0) return;

  // 显示正在计算
  displayMessage("Calculating...");

  // 设置全局角度模式
  g_angleMode = currentAngleMode;

  // 解析并计算
  Value result = evaluateExpression(inputBuffer, currentMode);
  ErrorCode err = getLastError();

  // 显示结果
  if (err != ERR_NONE) {
    displayError(err);
    Serial.print(F("Error: "));
    char errMsg[32];
    getLastErrorMessage(errMsg, sizeof(errMsg));
    Serial.println(errMsg);
  } else {
    displayResult(result);

    // 串口输出结果
    Serial.print(F("Input: "));
    Serial.println(inputBuffer);
    Serial.print(F("Result: "));

    switch (result.type) {
      case VAL_NUMBER:
        Serial.println(result.number, DISPLAY_PRECISION);
        break;
      case VAL_COMPLEX: {
        char buf[64];
        result.complex.toString(buf, sizeof(buf), DISPLAY_PRECISION);
        Serial.println(buf);
        break;
      }
      case VAL_MATRIX: {
        char buf[16];
        result.matrix.toString(buf, sizeof(buf));
        Serial.println(buf);
        result.matrix.print();
        break;
      }
      default:
        Serial.println(F("Unknown"));
    }

    // 将结果存入 Ans 变量（A）
    storeVariable('A', result);
  }

  // 清空输入，准备下一次计算
  clearInput();
}

// ============================================================
// 输入缓冲区管理
// ============================================================
void clearInput() {
  inputPos = 0;
  inputBuffer[0] = '\0';
  displayInput("");
}

void deleteLastChar() {
  if (inputPos > 0) {
    inputPos--;
    inputBuffer[inputPos] = '\0';
    displayInput(inputBuffer);
  }
}

void appendToInput(char c) {
  if (inputPos < MAX_EXPR_LENGTH - 1) {
    inputBuffer[inputPos++] = c;
    inputBuffer[inputPos] = '\0';
    displayInput(inputBuffer);
  }
}

void appendStringToInput(const char* str) {
  uint8_t len = strlen(str);
  for (uint8_t i = 0; i < len && inputPos < MAX_EXPR_LENGTH - 1; i++) {
    inputBuffer[inputPos++] = str[i];
  }
  inputBuffer[inputPos] = '\0';
  displayInput(inputBuffer);
}

// ============================================================
// 模式切换
// ============================================================
void toggleMode() {
  switch (currentMode) {
    case MODE_NORMAL:
      currentMode = MODE_MATRIX;
      displayMessage("Matrix Mode");
      break;
    case MODE_MATRIX:
      currentMode = MODE_COMPLEX;
      displayMessage("Complex Mode");
      break;
    case MODE_COMPLEX:
      currentMode = MODE_NORMAL;
      displayMessage("Normal Mode");
      break;
    default:
      currentMode = MODE_NORMAL;
      displayMessage("Normal Mode");
      break;
  }
  displayStatus(currentMode, currentAngleMode);
}

void toggleAngleMode() {
  if (currentAngleMode == DEGREE) {
    currentAngleMode = RADIAN;
    displayMessage("Radian Mode");
    Serial.println(F("Angle mode: RADIAN"));
  } else {
    currentAngleMode = DEGREE;
    displayMessage("Degree Mode");
    Serial.println(F("Angle mode: DEGREE"));
  }
  displayStatus(currentMode, currentAngleMode);
}

// ============================================================
// 帮助信息
// ============================================================
void showHelp() {
  Serial.println(F("\n=== Arduino Calculator Help ==="));
  Serial.println(F("Keyboard Layout (5x5):"));
  Serial.println(F("  [7][8][9][/][C]  C=Clear"));
  Serial.println(F("  [4][5][6][*][D]  D=Delete"));
  Serial.println(F("  [1][2][3][-][S]  S=Shift/2nd"));
  Serial.println(F("  [0][.][=][+][A]  A=Alpha"));
  Serial.println(F("  [(][)][^][M][E]  M=Mode, E=Enter"));
  Serial.println();
  Serial.println(F("Modes:"));
  Serial.println(F("  NORMAL: Basic arithmetic"));
  Serial.println(F("  2ND:    Scientific functions"));
  Serial.println(F("  ALPHA:  Variables (A-J)"));
  Serial.println(F("  MATRIX: Matrix operations"));
  Serial.println(F("  COMPLEX: Complex numbers"));
  Serial.println();
  Serial.println(F("Examples:"));
  Serial.println(F("  sin(30)     -> 0.5 (in DEG mode)"));
  Serial.println(F("  sqrt(16)    -> 4"));
  Serial.println(F("  2^10        -> 1024"));
  Serial.println(F("  fact(5)     -> 120"));
  Serial.println(F("  [1,2;3,4]   -> 2x2 matrix"));
  Serial.println(F("  3+4i        -> complex (use ALPHA+i)"));
  Serial.println();
  Serial.println(F("Serial commands:"));
  Serial.println(F("  c = clear input"));
  Serial.println(F("  a = toggle angle mode"));
  Serial.println(F("  h = show this help"));
  Serial.println(F("================================\n"));
}
