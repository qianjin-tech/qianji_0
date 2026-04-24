/**
 * Keyboard Input Handler Implementation (4x4 Matrix)
 * Supports FN layer switching without key conflicts
 */

#include "keyboard.h"

// ============================================================
// 行引脚和列引脚数组 (4x4)
// ============================================================
static const uint8_t ROW_PINS[4] = {KEY_ROW0, KEY_ROW1, KEY_ROW2, KEY_ROW3};
static const uint8_t COL_PINS[4] = {KEY_COL0, KEY_COL1, KEY_COL2, KEY_COL3};

// ============================================================
// 消抖与状态
// ============================================================
static char lastKey = '\0';
static unsigned long lastKeyTime = 0;
static unsigned long fnPressTime = 0;
static const unsigned long DEBOUNCE_MS = 50;
static const unsigned long REPEAT_DELAY_MS = 500;
static const unsigned long REPEAT_RATE_MS = 150;
static const unsigned long FN_DOUBLE_PRESS_MS = 400;  // 双击FN的时间窗口

// ============================================================
// 初始化键盘引脚
// ============================================================
void keyboardInit() {
  // 行引脚设为输出
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(ROW_PINS[i], OUTPUT);
    digitalWrite(ROW_PINS[i], HIGH);
  }
  // 列引脚设为上拉输入
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(COL_PINS[i], INPUT_PULLUP);
  }
}

// ============================================================
// 扫描键盘（返回基础层字符）
// ============================================================
char keyboardScan() {
  for (uint8_t row = 0; row < 4; row++) {
    // 将当前行设为低电平，其他行高电平
    for (uint8_t r = 0; r < 4; r++) {
      digitalWrite(ROW_PINS[r], (r == row) ? LOW : HIGH);
    }

    delayMicroseconds(10);  // 稳定时间

    // 扫描列
    for (uint8_t col = 0; col < 4; col++) {
      if (digitalRead(COL_PINS[col]) == LOW) {
        // 恢复所有行高电平
        for (uint8_t r = 0; r < 4; r++) {
          digitalWrite(ROW_PINS[r], HIGH);
        }

        // 返回基础层按键字符
        return pgm_read_byte(&KEY_MAP_NORMAL[row][col]);
      }
    }
  }

  // 恢复所有行
  for (uint8_t r = 0; r < 4; r++) {
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
      if (repeatTime % REPEAT_RATE_MS < 20) {
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
// 根据FN层状态获取实际按键字符
// ============================================================
char getKeyForFnLayer(char baseKey, FnLayerState fnState) {
  // 查找 baseKey 在基础映射中的位置
  for (uint8_t row = 0; row < 4; row++) {
    for (uint8_t col = 0; col < 4; col++) {
      if (pgm_read_byte(&KEY_MAP_NORMAL[row][col]) == baseKey) {
        switch (fnState) {
          case FN_ONCE:
          case FN_LOCK:
            return pgm_read_byte(&KEY_MAP_FN[row][col]);
          case FN2_LOCK:
            return pgm_read_byte(&KEY_MAP_FN2[row][col]);
          default:
            return baseKey;
        }
      }
    }
  }
  return baseKey;
}

// ============================================================
// 获取FN层状态名称
// ============================================================
const char* getFnLayerName(FnLayerState state) {
  switch (state) {
    case FN_OFF:    return "";
    case FN_ONCE:   return "FN";
    case FN_LOCK:   return "FN*";
    case FN2_LOCK:  return "FN2";
    default:        return "";
  }
}
