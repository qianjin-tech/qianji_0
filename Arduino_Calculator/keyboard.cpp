/**
 * Keyboard Input Handler Implementation
 */

#include "keyboard.h"

// ============================================================
// 行引脚和列引脚数组
// ============================================================
static const uint8_t ROW_PINS[5] = {KEY_ROW0, KEY_ROW1, KEY_ROW2, KEY_ROW3, KEY_ROW4};
static const uint8_t COL_PINS[5] = {KEY_COL0, KEY_COL1, KEY_COL2, KEY_COL3, KEY_COL4};

// ============================================================
// 消抖状态
// ============================================================
static char lastKey = '\0';
static unsigned long lastKeyTime = 0;
static const unsigned long DEBOUNCE_MS = 50;
static const unsigned long REPEAT_DELAY_MS = 500;
static const unsigned long REPEAT_RATE_MS = 150;

// ============================================================
// 初始化键盘引脚
// ============================================================
void keyboardInit() {
  // 行引脚设为输出
  for (uint8_t i = 0; i < 5; i++) {
    pinMode(ROW_PINS[i], OUTPUT);
    digitalWrite(ROW_PINS[i], HIGH);
  }
  // 列引脚设为上拉输入
  for (uint8_t i = 0; i < 5; i++) {
    pinMode(COL_PINS[i], INPUT_PULLUP);
  }
}

// ============================================================
// 扫描键盘
// ============================================================
char keyboardScan() {
  for (uint8_t row = 0; row < 5; row++) {
    // 将当前行设为低电平
    for (uint8_t r = 0; r < 5; r++) {
      digitalWrite(ROW_PINS[r], (r == row) ? LOW : HIGH);
    }

    delayMicroseconds(10);  // 稳定时间

    // 扫描列
    for (uint8_t col = 0; col < 5; col++) {
      if (digitalRead(COL_PINS[col]) == LOW) {
        // 恢复行电平
        for (uint8_t r = 0; r < 5; r++) {
          digitalWrite(ROW_PINS[r], HIGH);
        }

        // 返回基础按键字符（默认模式）
        return pgm_read_byte(&KEY_MAP_NORMAL[row][col]);
      }
    }
  }

  // 恢复所有行
  for (uint8_t r = 0; r < 5; r++) {
    digitalWrite(ROW_PINS[r], HIGH);
  }

  return '\0';
}

// ============================================================
// 获取按键事件（带消抖和重复）
// ============================================================
KeyEvent getKeyEvent(char& keyChar) {
  char key = keyboardScan();
  unsigned long now = millis();

  if (key == '\0') {
    if (lastKey != '\0') {
      lastKey = '\0';
      return KEY_RELEASED;
    }
    return KEY_NONE;
  }

  if (key != lastKey) {
    // 新按键
    if (now - lastKeyTime > DEBOUNCE_MS) {
      lastKey = key;
      lastKeyTime = now;
      keyChar = key;
      return KEY_PRESSED;
    }
  } else {
    // 持续按住
    unsigned long holdTime = now - lastKeyTime;
    if (holdTime > REPEAT_DELAY_MS) {
      unsigned long repeatTime = holdTime - REPEAT_DELAY_MS;
      if (repeatTime % REPEAT_RATE_MS < 20) {  // 在重复窗口内
        keyChar = key;
        return KEY_HOLD;
      }
    }
  }

  return KEY_NONE;
}

// ============================================================
// 等待按键按下（阻塞）
// ============================================================
char waitForKey() {
  char key;
  while (true) {
    KeyEvent evt = getKeyEvent(key);
    if (evt == KEY_PRESSED) {
      return key;
    }
    delay(10);
  }
}

// ============================================================
// 根据当前模式获取按键映射
// ============================================================
char getKeyForMode(char baseKey, CalcMode mode) {
  // 查找 baseKey 在默认映射中的位置
  for (uint8_t row = 0; row < 5; row++) {
    for (uint8_t col = 0; col < 5; col++) {
      if (pgm_read_byte(&KEY_MAP_NORMAL[row][col]) == baseKey) {
        switch (mode) {
          case MODE_2ND:
            return pgm_read_byte(&KEY_MAP_2ND[row][col]);
          case MODE_ALPHA:
            return pgm_read_byte(&KEY_MAP_ALPHA[row][col]);
          default:
            return baseKey;
        }
      }
    }
  }
  return baseKey;
}
