# FINITE STATE MACHINE

## 简介

[有限状态机](https://zh.wikipedia.org/wiki/%E6%9C%89%E9%99%90%E7%8A%B6%E6%80%81%E6%9C%BA)（finite-state machine）是表示有限个状态以及在这些状态之间的转移和动作等行为的数学计算模型。

## 概述

### 使用

这是一个仅含头文件的库，使用时仅需要引用把 `include` 文件夹内的头文件即可。

在项目的具体使用方法，详见 [1nchy/project_template](https://github.com/1nchy/project_template)。

### 示例

我们以 tcp 拥塞控制的有限状态机为例，详情见 [TCP 拥塞控制](./doc/tcp_congestion_control/usage.md)。

### 设计

该有限状态机的设计，包含了状态进入/退出动作、事件响应两种主要的特性。

详情见 [设计文档](./doc/design.md)。

## 项目结构

~~~txt
finite_state_machine
├── .gitignore
├── CMakeLists.txt
├── LICENSE.md
├── README.md
├── doc                         # 项目文档
│   ├── design.md               # 设计文档
│   └── tcp_congestion_control
│       └── usage.md            # 使用说明
├── include                     # 头文件（对外使用）
│   └── ...
└── example                     # 示例
    └── tcp_congestion_control  # tcp 拥塞控制示例
        └── ...
~~~