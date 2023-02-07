# qcc 一个简单的RISCV64架构C语言编译器
本仓库结合本人之前编译原理课程设计知识，参考学习了 [sunshaoce](https://github.com/sunshaoce) 和 [ksco](https://github.com/ksco) 的 [rvcc](https://github.com/sunshaoce/rvcc) 项目，尝试从零开始构建一个 RISC-V64 架构的C语言编译器。

## 学习资料
主要包括了[C11规范标准文档](doc/n1570.pdf)、[RISC-V标准文档](doc/RISC-V-Reader-Chinese-v2p12017.pdf)、编译原理(龙书)，随着学习深入会不断添加到 doc 文件夹

## 改进思路
降低词法分析器、语法分析器、代码生成器之间的耦合
- 修改原有 Token 和 Node 结构体，增加更多成员变量
- 在 Token 和 Node 两大结构体基础上，进一步抽象设计出 Lexer、Parser、Codegener 结构体，将原有的 static 变量封装入其中，增加了文件信息相关字段，便于大家调用查看使用
- 调整重写部分函数

优化结构体空间结构
- 使用 union 成员变量节省结构体空间
- 调整结构体成员变量顺序，减少由于对齐造成的结构体内存空间浪费

优化文件结构
- 调整文件位置，使项目文件结构更清晰

优化报错信息
- 减少冗余输出，仅输出出错行&错误信息

代码可读性
- 修改&增加了部分中文注释，使得其更加适合初学者阅读

## 项目进度
|编号|计划目标|完成情况|
|:----:|:----:|:----:|
| [1] | 实现整数正负、加减乘除、比较关系、表达式编译功能 | finish |
| [2] | 支持用文件名编译文件 | finish |
| [3] | 修改test.sh文件：增加cmake构建命令、增加注释、改进易用性 | finish |
| [4] | 支持 ";" 分割表达式 | finish |
| [5] | 支持多字母本地变量 | finish |
| [6] | 支持 return 语句 | coding |


## 学习&&使用方法
### 环境准备 ( x86 Ubuntu22.04 )
1. 下载并编译 [riscv-gnu-toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain) 
2. 设置环境变量环境变量

### 使用方法
请根据运行机实际情况调整 test.sh 中的内容，通过修改 /test/test 文件更改测试用例，在该文件夹根目录下执行以下命令

    ./test.sh


