# qianjin-tech 的 GitHub 仓库

> 欢迎来到我的代码世界！这里记录着我的学习历程和项目实践。

---

## 关于我

你好，我是 **qianjin-tech**，一名热爱技术、正在不断学习的开发者。
这个仓库是我发现 GitHub 这块"新大陆"后的第一个家，
我会在这里存放各种有趣的项目和学习笔记。

---

## 项目列表

### 1. Arduino Advanced Calculator v2.0 ⭐

一个功能强大的 Arduino 科学计算器，**包含硬件固件和电脑端控制界面**。

**v2.0 新特性：**
- **4x4 矩阵键盘**：FN 层切换，无按键冲突（替代原来的 5x5）
- **电脑端界面**：Python + Tkinter，美观暗色主题
- **串口通信**：PC 通过 USB 连接控制 Arduino
- **双模式运行**：可独立在 PC 上计算，也可连接硬件使用

**功能特性：**
- 基础四则运算（+、-、*、/、^）
- 科学函数（sin、cos、tan、log、ln、exp、sqrt、阶乘等）
- 复数运算（四则运算、模、辐角、三角函数）
- 矩阵运算（加减乘、转置、行列式、求逆）
- 变量存储（A-J）
- 计算历史记录

**硬件需求：**
- Arduino Uno/Nano
- LCD2004 (I2C)
- 4x4 矩阵键盘

📁 [进入项目](./Arduino_Calculator/)

---

## 快速开始

### 克隆仓库
```bash
git clone https://github.com/qianjin-tech/qianji_0.git
cd qianji_0
```

### 硬件端（Arduino）
```bash
# 用 Arduino IDE 打开固件
cd Arduino_Calculator/Arduino
# 打开 Arduino_Calculator.ino，编译上传
```

### 电脑端（Python）
```bash
cd Arduino_Calculator/PC_Interface
pip install -r requirements.txt
python calculator_gui.py
```

---

## 学习计划

- [x] 学习 Markdown 语法
- [x] 创建第一个 GitHub 仓库
- [x] 完成 Arduino 计算器 v1.0
- [x] 完成 Arduino 计算器 v2.0（4x4键盘 + PC界面）
- [ ] 学习更多嵌入式开发
- [ ] 探索开源硬件项目

---

## 联系我

如果你对我的项目感兴趣，或者有任何建议，欢迎通过 GitHub 与我交流！

---

*Last updated: 2026-04-24*
