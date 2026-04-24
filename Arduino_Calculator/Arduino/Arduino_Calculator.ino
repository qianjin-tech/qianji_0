/**
 * Arduino Advanced Calculator v2.0
 * ============================================
 * A powerful scientific calculator for Arduino Uno/Nano
 * 
 * v2.0 Updates:
 *   - Changed from 5x5 to 4x4 matrix keyboard
 *   - FN layer system for function access (no key conflicts)
 *   - Serial protocol for PC interface communication
 *
 * Hardware:
 *   - Arduino Uno/Nano
 *   - LCD2004 (I2C, address 0x27)
 *   - 4x4 matrix keypad
 *
 * Author: qianjin-tech
 * Version: v2.0
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
static FnLayerState fnState = FN_OFF;
static unsigned long lastFnPressTime = 0;
static const unsigned long FN_DOUBLE_PRESS_MS = 400;

// 串口协议缓冲区
static char serialBuffer[MAX_EXPR_LENGTH];
static uint8_t serialPos = 0;

// ============================================================
// 函数前向声明
// ============================================================
void handleKey(char key);
void handleFnKey();
void processInput();
void clearInput();
void deleteLastChar();
void appendToInput(char c);
void appendStringToInput(const char* str);
void toggleCalcMode();
void toggleAngleMode();
void showHelp();

// 串口协议处理
void processSerialLine();

// ============================================================
// 初始化
// ============================================================
void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }

  Serial.println(F("================================"));
  Serial.println(F("  Arduino Advanced Calculator"));
  Serial.print(F("  Version: "));
  Serial.println(F(CALC_VERSION));
  Serial.println(F("================================"));
  Serial.println();
  Serial.println(F("Initializing..."));

  keyboardInit();
  displayInit();

  clearInput();
  clearVariables();
  clearMatrices();
  fnState = FN_OFF;

  displayStatus(currentMode, currentAngleMode);
  displayMessage("Ready v2.0");

  Serial.println(F("Calculator ready!"));
  Serial.println(F("Serial: >CALC expr  >MODE DEG/RAD  >STORE A val  >RECALL A"));
}

// ============================================================
// 主循环
// ============================================================
void loop() {
  // 处理串口输入
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (serialPos > 0) {
        serialBuffer[serialPos] = '\0';
        processSerialLine();
        serialPos = 0;
      }
    } else if (serialPos < MAX_EXPR_LENGTH - 1) {
      serialBuffer[serialPos++] = c;
    }
  }

  // 处理键盘输入
  char key;
  KeyEvent evt = getKeyEvent(key);

  if (evt == KEY_PRESSED) {
    handleKey(key);
  }

  delay(10);
}

// ============================================================
// 串口协议处理
// ============================================================
void processSerialLine() {
  char* cmd = serialBuffer;
  while (*cmd == ' ') cmd++;

  if (strncmp(cmd, ">CALC ", 6) == 0) {
    char* expr = cmd + 6;
    g_angleMode = currentAngleMode;
    Value result = evaluateExpression(expr, currentMode);
    ErrorCode err = getLastError();

    if (err != ERR_NONE) {
      char errMsg[32];
      getLastErrorMessage(errMsg, sizeof(errMsg));
      Serial.print(F("<ERR "));
      Serial.println(errMsg);
    } else {
      Serial.print(F("<OK "));
      if (result.type == VAL_NUMBER) {
        Serial.println(result.number, DISPLAY_PRECISION);
      } else if (result.type == VAL_COMPLEX) {
        char buf[64];
        result.complex.toString(buf, sizeof(buf), DISPLAY_PRECISION);
        Serial.println(buf);
      } else if (result.type == VAL_MATRIX) {
        char buf[16];
        result.matrix.toString(buf, sizeof(buf));
        Serial.println(buf);
      }
      storeVariable('A', result);
    }
  }
  else if (strncmp(cmd, ">MODE ", 6) == 0) {
    char* mode = cmd + 6;
    if (strncmp(mode, "DEG", 3) == 0) {
      currentAngleMode = DEGREE;
      Serial.println(F("<OK DEG"));
    } else if (strncmp(mode, "RAD", 3) == 0) {
      currentAngleMode = RADIAN;
      Serial.println(F("<OK RAD"));
    } else {
      Serial.println(F("<ERR Unknown mode"));
    }
    displayStatus(currentMode, currentAngleMode);
  }
  else if (strncmp(cmd, ">STORE ", 7) == 0) {
    char varName = cmd[7];
    if (varName >= 'A' && varName <= 'J') {
      double val = atof(cmd + 9);
      Value v;
      v.type = VAL_NUMBER;
      v.number = val;
      storeVariable(varName, v);
      Serial.print(F("<OK "));
      Serial.println(varName);
    } else {
      Serial.println(F("<ERR Invalid variable"));
    }
  }
  else if (strncmp(cmd, ">RECALL ", 8) == 0) {
    char varName = cmd[8];
    Value v;
    if (recallVariable(varName, v)) {
      Serial.print(F("<OK "));
      Serial.println(v.number, DISPLAY_PRECISION);
    } else {
      Serial.println(F("<ERR Empty"));
    }
  }
  else if (strcmp(cmd, ">RESET") == 0) {
    clearInput();
    clearVariables();
    clearMatrices();
    currentMode = MODE_NORMAL;
    currentAngleMode = DEGREE;
    fnState = FN_OFF;
    displayClear();
    displayStatus(currentMode, currentAngleMode);
    Serial.println(F("<OK RESET"));
  }
  else if (strcmp(cmd, ">STATUS") == 0) {
    Serial.print(F("<OK MODE="));
    Serial.print(currentMode);
    Serial.print(F(" ANGLE="));
    Serial.print(currentAngleMode);
    Serial.print(F(" FN="));
    Serial.println(fnState);
  }
  else {
    Serial.println(F("<ERR Unknown command"));
  }
}

// ============================================================
// 按键处理
// ============================================================
void handleKey(char key) {
  // FN键特殊处理（基础层的 '+' 作为FN键）
  if (key == '+') {
    handleFnKey();
    return;
  }

  // 根据FN层状态转换按键
  char actualKey = getKeyForFnLayer(key, fnState);

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

    case KEY_CHAR_MODE:
      toggleCalcMode();
      return;

    case KEY_CHAR_ENTER:
      processInput();
      return;
  }

  // 将按键字符添加到输入
  appendToInput(actualKey);

  // FN_ONCE模式下，使用一次后自动关闭
  if (fnState == FN_ONCE) {
    fnState = FN_OFF;
    displayStatus(currentMode, currentAngleMode);
  }
}

// ============================================================
// FN键处理（双击检测）
// ============================================================
void handleFnKey() {
  unsigned long now = millis();

  if (fnState == FN_OFF) {
    // 第一次按FN
    if (now - lastFnPressTime < FN_DOUBLE_PRESS_MS) {
      // 双击 -> 进入FN2_LOCK
      fnState = FN2_LOCK;
      displayMessage("FN2 Lock");
    } else {
      // 单击 -> FN_ONCE（下一键使用FN层）
      fnState = FN_ONCE;
    }
    lastFnPressTime = now;
  }
  else if (fnState == FN_ONCE) {
    // 在FN_ONCE状态下再按FN -> 进入FN_LOCK
    fnState = FN_LOCK;
    displayMessage("FN Lock");
  }
  else if (fnState == FN_LOCK) {
    // 在FN_LOCK状态下按FN -> 进入FN2_LOCK
    fnState = FN2_LOCK;
    displayMessage("FN2 Lock");
  }
  else if (fnState == FN2_LOCK) {
    // 在FN2_LOCK状态下按FN -> 关闭
    fnState = FN_OFF;
    displayMessage("FN Off");
  }

  displayStatus(currentMode, currentAngleMode);
}

// ============================================================
// 处理输入并计算
// ============================================================
void processInput() {
  if (inputPos == 0) return;

  displayMessage("Calculating...");
  g_angleMode = currentAngleMode;

  Value result = evaluateExpression(inputBuffer, currentMode);
  ErrorCode err = getLastError();

  if (err != ERR_NONE) {
    displayError(err);
    Serial.print(F("Error: "));
    char errMsg[32];
    getLastErrorMessage(errMsg, sizeof(errMsg));
    Serial.println(errMsg);
  } else {
    displayResult(result);

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

    storeVariable('A', result);
  }

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
void toggleCalcMode() {
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
  Serial.println(F("\n=== Arduino Calculator v2.0 Help ==="));
  Serial.println(F("Keyboard Layout (4x4):"));
  Serial.println(F("  [7][8][9][/]"));
  Serial.println(F("  [4][5][6][*]"));
  Serial.println(F("  [1][2][3][-]"));
  Serial.println(F("  [0][.][=][+]  (+ is FN key)"));
  Serial.println();
  Serial.println(F("FN Layer (press + once):"));
  Serial.println(F("  [sin][cos][tan][log]"));
  Serial.println(F("  [asin][acos][atan][ln]"));
  Serial.println(F("  [exp][sqrt][cbrt][^]"));
  Serial.println(F("  [(][)][DEL][FN]"));
  Serial.println();
  Serial.println(F("FN2 Layer (press + twice):"));
  Serial.println(F("  [A][B][C][i]"));
  Serial.println(F("  [G][H][J][K]"));
  Serial.println(F("  [[][ ]; [M]"));
  Serial.println(F("  [N][O][CLR][MODE]"));
  Serial.println();
  Serial.println(F("Serial commands:"));
  Serial.println(F("  >CALC expr   - Calculate expression"));
  Serial.println(F("  >MODE DEG    - Set degree mode"));
  Serial.println(F("  >MODE RAD    - Set radian mode"));
  Serial.println(F("  >STORE A val - Store value to variable"));
  Serial.println(F("  >RECALL A    - Recall variable"));
  Serial.println(F("  >RESET       - Reset calculator"));
  Serial.println(F("  >STATUS      - Show status"));
  Serial.println(F("================================\n"));
}
