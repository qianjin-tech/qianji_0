/**
 * Display Manager Implementation
 */

#include "display.h"

// ============================================================
// LCD 对象
// ============================================================
static LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);

// ============================================================
// 初始化
// ============================================================
void displayInit() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Arduino Calc ");
  lcd.print(CALC_VERSION);
  lcd.setCursor(0, 1);
  lcd.print("Ready...");
  delay(1000);
  lcd.clear();
}

void displayClear() {
  lcd.clear();
}

// ============================================================
// 显示输入表达式（自动截断或滚动）
// ============================================================
void displayInput(const char* expr) {
  lcd.setCursor(0, DISP_ROW_INPUT);
  uint8_t len = strlen(expr);
  if (len <= LCD_COLS) {
    lcd.print(expr);
    // 清除剩余空间
    for (uint8_t i = len; i < LCD_COLS; i++) {
      lcd.print(' ');
    }
  } else {
    // 显示最后 LCD_COLS 个字符，前面加省略号
    lcd.print("...");
    lcd.print(expr + len - (LCD_COLS - 3));
  }
}

// ============================================================
// 显示计算结果
// ============================================================
void displayResult(const Value& value) {
  lcd.setCursor(0, DISP_ROW_RESULT);

  char buffer[LCD_COLS + 1];
  for (uint8_t i = 0; i < LCD_COLS; i++) buffer[i] = ' ';
  buffer[LCD_COLS] = '\0';

  lcd.print("=");

  switch (value.type) {
    case VAL_NUMBER: {
      // 格式化数字显示
      if (isnan(value.number)) {
        lcd.print("Error");
      } else if (isinf(value.number)) {
        lcd.print(value.number > 0 ? "Infinity" : "-Infinity");
      } else {
        // 根据大小自动选择格式
        double absVal = fabs(value.number);
        if (absVal == 0.0) {
          lcd.print("0");
        } else if (absVal >= 1e6 || (absVal < 1e-4 && absVal > 0)) {
          // 科学计数法
          char fmt[16];
          snprintf(fmt, sizeof(fmt), "%%.%de", DISPLAY_PRECISION - 1);
          snprintf(buffer, sizeof(buffer), fmt, value.number);
          lcd.print(buffer);
        } else {
          // 普通格式
          char fmt[16];
          snprintf(fmt, sizeof(fmt), "%%.%df", DISPLAY_PRECISION);
          snprintf(buffer, sizeof(buffer), fmt, value.number);
          // 去除末尾的0
          uint8_t blen = strlen(buffer);
          while (blen > 0 && buffer[blen - 1] == '0') {
            buffer[blen - 1] = '\0';
            blen--;
          }
          if (blen > 0 && buffer[blen - 1] == '.') {
            buffer[blen - 1] = '\0';
          }
          lcd.print(buffer);
        }
      }
      break;
    }

    case VAL_COMPLEX: {
      value.complex.toString(buffer, sizeof(buffer), DISPLAY_PRECISION);
      if (strlen(buffer) > LCD_COLS - 1) {
        lcd.print("...");
        lcd.print(buffer + strlen(buffer) - (LCD_COLS - 4));
      } else {
        lcd.print(buffer);
      }
      break;
    }

    case VAL_MATRIX: {
      char matStr[16];
      value.matrix.toString(matStr, sizeof(matStr));
      lcd.print(matStr);
      lcd.print(" [M]");
      break;
    }

    default:
      lcd.print("---");
  }
}

// ============================================================
// 显示状态信息
// ============================================================
void displayStatus(CalcMode mode, AngleMode angleMode) {
  lcd.setCursor(0, DISP_ROW_STATUS);

  // 模式指示
  const char* modeStr;
  switch (mode) {
    case MODE_NORMAL:  modeStr = "NORM"; break;
    case MODE_2ND:     modeStr = "2ND";  break;
    case MODE_ALPHA:   modeStr = "ALPH"; break;
    case MODE_MATRIX:  modeStr = "MAT";  break;
    case MODE_COMPLEX: modeStr = "CPLX"; break;
    default:           modeStr = "???";  break;
  }

  // 角度模式
  const char* angleStr = (angleMode == DEGREE) ? "DEG" : "RAD";

  char buffer[LCD_COLS + 1];
  snprintf(buffer, sizeof(buffer), "[%s] [%s]", modeStr, angleStr);

  lcd.print(buffer);
  // 清除剩余空间
  for (uint8_t i = strlen(buffer); i < LCD_COLS; i++) {
    lcd.print(' ');
  }
}

// ============================================================
// 显示错误信息
// ============================================================
void displayError(ErrorCode err) {
  lcd.setCursor(0, DISP_ROW_MESSAGE);
  char errMsg[LCD_COLS + 1];
  getErrorMessage(err, errMsg, sizeof(errMsg));

  lcd.print("ERR:");
  lcd.print(errMsg);

  // 清除剩余空间
  for (uint8_t i = 4 + strlen(errMsg); i < LCD_COLS; i++) {
    lcd.print(' ');
  }
}

// ============================================================
// 显示提示消息
// ============================================================
void displayMessage(const char* msg) {
  lcd.setCursor(0, DISP_ROW_MESSAGE);
  uint8_t len = strlen(msg);
  if (len > LCD_COLS) len = LCD_COLS;
  lcd.print(msg);
  for (uint8_t i = len; i < LCD_COLS; i++) {
    lcd.print(' ');
  }
}

// ============================================================
// 显示矩阵内容
// ============================================================
void displayMatrix(const Matrix& mat, uint8_t page) {
  lcd.clear();
  lcd.setCursor(0, 0);
  char header[21];
  snprintf(header, sizeof(header), "Matrix %dx%d", mat.rows, mat.cols);
  lcd.print(header);

  uint8_t startRow = page * 2;
  for (uint8_t i = 0; i < 2 && (startRow + i) < mat.rows; i++) {
    lcd.setCursor(0, i + 1);
    uint8_t r = startRow + i;
    for (uint8_t c = 0; c < mat.cols && c < 4; c++) {
      char buf[8];
      snprintf(buf, sizeof(buf), "%6.2f", mat.data[r][c]);
      lcd.print(buf);
      if (c < mat.cols - 1) lcd.print(" ");
    }
  }
}

// ============================================================
// 显示复数
// ============================================================
void displayComplex(const Complex& c) {
  lcd.setCursor(0, DISP_ROW_RESULT);
  char buffer[LCD_COLS + 1];
  c.toString(buffer, sizeof(buffer), DISPLAY_PRECISION);
  lcd.print(buffer);
}

// ============================================================
// 滚动显示长文本（简化版）
// ============================================================
void displayScrollText(uint8_t row, const char* text) {
  // 简化实现：只显示前 LCD_COLS 个字符
  lcd.setCursor(0, row);
  uint8_t len = strlen(text);
  if (len > LCD_COLS) len = LCD_COLS;
  for (uint8_t i = 0; i < len; i++) {
    lcd.print(text[i]);
  }
  for (uint8_t i = len; i < LCD_COLS; i++) {
    lcd.print(' ');
  }
}

// ============================================================
// 光标控制
// ============================================================
void displaySetCursor(uint8_t col, uint8_t row) {
  lcd.setCursor(col, row);
}

// ============================================================
// 背光控制
// ============================================================
void displayBacklight(bool on) {
  if (on) {
    lcd.backlight();
  } else {
    lcd.noBacklight();
  }
}
