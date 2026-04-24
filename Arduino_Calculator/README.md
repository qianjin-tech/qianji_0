# Arduino Advanced Calculator v2.0

一个功能强大的 Arduino 科学计算器项目，包含硬件固件和电脑端控制界面。

---

## 项目结构

```
Arduino_Calculator/
├── Arduino/                    # Arduino 硬件固件
│   ├── Arduino_Calculator.ino  # 主程序
│   ├── config.h                # 配置与常量
│   ├── scientific_math.h/cpp   # 科学计算库
│   ├── complex_number.h/cpp    # 复数运算
│   ├── matrix.h/cpp            # 矩阵运算
│   ├── expression_parser.h/cpp # 表达式解析器
│   ├── keyboard.h/cpp          # 4x4 键盘输入
│   ├── display.h/cpp           # LCD 显示管理
│   └── README.md               # 固件说明
│
├── PC_Interface/               # 电脑端控制界面
│   ├── calculator_gui.py       # 主程序入口
│   ├── themes.py               # UI 主题配置
│   ├── calc_engine.py          # 本地计算引擎
│   ├── serial_comm.py          # 串口通信模块
│   └── requirements.txt        # Python 依赖
│
└── README.md                   # 项目总说明
```

---

## 功能特性

### 硬件端（Arduino）
- **4x4 矩阵键盘**：FN 层切换，无按键冲突
- **LCD2004 显示**：四行显示输入、结果、状态
- **科学计算**：三角函数、对数、指数、阶乘等
- **复数运算**：四则运算、模、辐角、三角函数
- **矩阵运算**：加减乘、转置、行列式、求逆
- **串口通信**：支持 PC 端远程控制

### 电脑端（Python + Tkinter）
- **美观界面**：暗色主题，现代化 UI
- **虚拟键盘**：与硬件 4x4 键盘布局一致
- **本地计算**：不连接硬件也能使用
- **串口连接**：通过 USB 连接 Arduino
- **历史记录**：保存计算历史，可点击复用
- **多面板**：科学函数、复数、矩阵、历史

---

## 快速开始

### 硬件端

1. 连接 LCD2004 和 4x4 键盘到 Arduino
2. 用 Arduino IDE 打开 `Arduino/Arduino_Calculator.ino`
3. 安装 `LiquidCrystal I2C` 库
4. 编译上传

### 电脑端

```bash
# 进入 PC 端目录
cd PC_Interface

# 安装依赖
pip install -r requirements.txt

# 运行程序
python calculator_gui.py
```

---

## 键盘操作

| 操作 | 效果 |
|------|------|
| 按 `+` 一次 | 进入 FN 层（下一键使用科学函数）|
| 按 `+` 两次 | 进入 FN2 层（变量和模式）|
| FN + `=` | 删除 |
| FN2 + `=` | 清空 |
| FN2 + `+` | 切换计算模式 |

---

## 串口命令

```
>CALC sin(30)+sqrt(16)    # 计算
>MODE DEG                 # 角度制
>MODE RAD                 # 弧度制
>STORE A 3.14             # 存变量
>RECALL A                 # 读变量
>RESET                    # 重置
>STATUS                   # 状态
```

---

## 作者

qianjin-tech

## 版本

- **v2.0** - 4x4键盘 + PC界面 + 串口协议
- **v1.0** - 初始版本（5x5键盘）
