# Cplusplus-Learning

C++ 学习笔记与练习代码，按学习阶段整理。

## 学习目录

| 顺序 | 主题 | 笔记与代码 |
| --- | --- | --- |
| 1 | C++ 入门：命名空间、输入输出、缺省参数、函数重载、引用、inline、nullptr | [CppOutset](CppOutset/README.md) |
| 2 | 类和对象：类的定义、构造与析构、运算符重载、初始化列表、static、友元、内部类等 | [Classes_And_Objects](Classes_And_Objects/README.md) |
| 3 | 日期类练习：日期运算、比较及输入输出 | [Date](Classes_And_Objects/Date/) |

## 目录结构

```text
Cplusplus-Learning/
├── README.md
├── CppOutset/
│   ├── README.md
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── Outset.h
│   └── Outset.cpp
└── Classes_And_Objects/
    ├── README.md
    ├── main.cpp
    ├── Object.h
    ├── Object.cpp
    ├── Date/
    │   ├── Date.h
    │   ├── Date.cpp
    │   └── test.cpp
    └── image/
        └── img.png
```

## 使用方式

克隆本仓库后，从上方学习目录进入对应章节阅读笔记和练习代码。

- `CppOutset` 保留原有 CMake 配置，要求 CMake 4.0 或更新版本，以及支持 C++20 的编译器。
- `Classes_And_Objects` 保留原有源码结构；`main.cpp` 是程序入口，`Date/test.cpp` 提供 `test()` 函数，没有独立的 `main()`。
- 每个章节保留自己的 `.gitignore`；仓库根目录忽略常见的本地构建产物和 IDE 配置。

## 仓库来源与历史

本仓库通过 Git subtree 导入以下仓库的 `main` 分支，未压缩提交，保留原始提交、作者和时间：

| 导入目录 | 来源仓库 | 导入时的提交 |
| --- | --- | --- |
| `CppOutset/` | [WeiYue-Xi/CppOutset](https://github.com/WeiYue-Xi/CppOutset) | `c0df33c` |
| `Classes_And_Objects/` | [WeiYue-Xi/Classes_And_Objects](https://github.com/WeiYue-Xi/Classes_And_Objects) | `67c1e80` |

源码和笔记保留在各自目录，类和对象笔记中的图片使用仓库内的相对路径。历史提交中的路径仍为原仓库路径，可通过 `git log --all --graph --oneline` 查看完整的合并历史。
