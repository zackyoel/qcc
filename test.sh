#!/bin/bash

# 构建文件
rebuild() {
    
  #################################################
  # 构建ccLearn
  cmake -Bbuild .
  cd ./build
  make
  cd ../
  
  
  #################################################
  # 检查 test 文件夹是否存在
  if [ ! -d "test" ]; then
  # 不存在则创建 test 文件夹
    mkdir test
  fi
}

#############################################################################

# 声明测试函数
assert() {
  #################################################
  # 程序运行的 期待值 为参数1
  expected="$1"
  # 输入值 为参数2
  input="$2"


  #################################################
  # 运行程序，传入期待值，将生成结果写入tmp.s汇编文件。
  # 如果运行不成功，则会执行exit退出。成功时会短路exit操作
  ./bin/qcc "$input" > ./test/tmp.s || exit


  #################################################
  # 编译rvcc产生的汇编文件，请根据实际情况选择指令

  # 使用 gcc 编译(RISC-V架构计算机)
  # gcc -o tmp tmp.s

  # 使用 RISC-V工具链 编译(非RISC-V机计算机)
  # 需要先将 RISCV 加入环境变量
  $RISCV/bin/riscv64-unknown-linux-gnu-gcc -static -o ./test/tmp.o ./test/tmp.s
  
  
  #################################################
  # 运行生成出来目标文件，请根据实际情况选择指令

  # 直接运行(RISC-V架构计算机)
  # ./tmp

  # qumu模拟运行(非RISC-V架构计算机)
  $RISCV/bin/qemu-riscv64 -L $RISCV/sysroot ./test/tmp.o

  # spike模拟运行
  # $RISCV/bin/spike --isa=rv64gc $RISCV/riscv64-unknown-linux-gnu/bin/pk ./tmp

  #################################################
  # 获取程序返回值，存入 实际值
  actual="$?"
  # 判断实际值，是否为预期值
  if [ "$actual" = "$expected" ]; then
    # 与预期值相同
    echo "$input => $actual"
  else
    # 与预期值不同
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi

}


# 构建文件
rebuild

# assert 期待值 输入值
# [1] 返回指定数值
assert 1 './test/tmp.c'

# 如果运行正常未提前退出，程序将显示OK
echo OK
