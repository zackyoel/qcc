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
assert 0 'return 0;'
assert 42 'return 42;'

# [1] 支持+ -运算符
assert 34 'return 12-34+56;'

# [1] 支持空格
assert 41 'return  12 + 34 - 5 ;'

# [1] 支持* / ()运算符
assert 47 'return 5+6*7;'
assert 15 'return 5*(9-6);'
assert 17 'return 1-8/(2*2)+3*6;'

# [1] 支持一元运算的+ -
assert 10 'return -10+20;'
assert 10 'return - -10;'
assert 10 'return - - +10;'
assert 48 'return ------12*+++++----++++++++++4;'

# [1] 支持条件运算符
assert 0 'return 0==1;'
assert 1 'return 42==42;'
assert 1 'return 0!=1;'
assert 0 'return 42!=42;'
assert 1 'return 0<1;'
assert 0 'return 1<1;'
assert 0 'return 2<1;'
assert 1 'return 0<=1;'
assert 1 'return 1<=1;'
assert 0 'return 2<=1;'
assert 1 'return 1>0;'
assert 0 'return 1>1;'
assert 0 'return 1>2;'
assert 1 'return 1>=0;'
assert 1 'return 1>=1;'
assert 0 'return 1>=2;'
assert 1 'return 5==2+3;'
assert 0 'return 6==4+3;'
assert 1 'return 0*9+5*2==4+4*(6/3)-2;'

# [4] 支持;分割语句
assert 3 '1; 2;return 3;'
assert 12 '12+23;12+99/3;return 78-66;'

# [5] 支持单字母变量
assert 3 'a=3;return a;'
assert 8 'a=3; z=5;return a+z;'
assert 6 'a=b=3;return a+b;'
assert 5 'a=3;b=4;a=1;return a+b;'
assert 3 'foo=3;return foo;'
assert 74 'foo2=70; bar4=4;return foo2+bar4;'

# [6] 支持return
assert 1 'return 1; 2; 3;'
assert 2 '1; return 2; 3;'
assert 3 '1; 2; return 3;'

# 如果运行正常未提前退出，程序将显示OK
echo OK
