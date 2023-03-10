#include "Compiler.h"

static unsigned int Depth;

/**
 * @brief 将 a0 压栈
 *
 */
static void push(void) {
  printf("    # 将 a0 压栈\n");
  printf("    addi sp, sp, -8\n");
  printf("    sd a0, 0(sp)\n");
  Depth++;
}

/**
 * @brief 弹栈到指定寄存器
 *
 * @param reg 目标寄存器 reg
 */
static void pop(char *reg) {
  printf("    # 弹栈入 %s \n", reg);
  printf("    ld %s, 0(sp)\n", reg);
  printf("    addi sp, sp, 8\n");
  Depth--;
}

/**
 * @brief 生成变量节点绝对地址
 *
 * @param node 变量节点
 */
static void genAddr(Node *node) {
  if (node->Kind == VAR) {
    // 偏移量是相对于fp的
    printf("    # 令 a0 = %s 的地址\n", node->Var->Name);
    printf("    addi a0, fp, %ld\n", node->Var->Offset);
    return;
  }

  error("Not Assignable\n");
}

/**
 * @brief 生成节点表达式语句值
 *
 * @param node 表达式语句节点
 */
static void genExpr(Node *node) {
  switch (node->Kind) {

  // 常数节点
  case NUM:
    printf("    # 将立即数 %d 写入 a0\n", node->Val);
    printf("    li a0, %d\n", node->Val);
    return;
  // 变量节点
  case VAR:
    genAddr(node);
    printf("    # 读取变量值\n");
    printf("    ld a0, 0(a0)\n");
    return;
  // 取反节点
  case NEG:
    genExpr(node->LHS);
    printf("    # a0 中的值取反放入 a0\n");
    printf("    neg a0, a0\n");
    return;
  // 赋值节点
  case ASSIGN:
    // 左部是左值，保存值到的地址
    genAddr(node->LHS);
    push();
    // 右部是右值，为表达式的值
    genExpr(node->RHS);
    pop("a1");
    printf("    # 将 a0 值 存入 a1 指向的内存地址\n");
    printf("    sd a0, 0(a1)\n");
    return;
  default:
    break;
  }

  // 递归至最右节点
  genExpr(node->RHS);
  // 右节点值压栈
  push();
  // 产生左节点值至 a0
  genExpr(node->LHS);
  // 弹栈右节点值至 a1
  pop("a1");

  // 运算符节点
  switch (node->Kind) {
  case ADD:
    printf("    # 令 a0 = a0 + a1\n");
    printf("    add a0, a0, a1\n");
    return;
  case SUB:
    printf("    # 令 a0 = a0 - a1\n");
    printf("    sub a0, a0, a1\n");
    return;
  case MUL:
    printf("    # 令 a0 = a0 * a1\n");
    printf("    mul a0, a0, a1\n");
    return;
  case DIV:
    printf("    # 令 a0 = a0 / a1\n");
    printf("    div a0, a0, a1\n");
    return;
  case EQ:
  case NE:
    printf("    # 令 a0 = a0 ^ a1\n");
    printf("    xor a0, a0, a1\n");
    if (node->Kind == EQ) {
      printf("    # 令 a0 = (a0 == 0)\n");
      printf("    seqz a0, a0\n");
    } else {
      printf("    # 令 a0 = (a0 != 0)\n");
      printf("    snez a0, a0\n");
    }
    return;
  case LT:
    printf("    # 令 a0 = a0 < a1\n");
    printf("    slt a0, a0, a1\n");
    return;
  case GT:
    printf("    # 令 a0 = a0 > a1\n");
    printf("    slt a0, a1, a0\n");
    return;
  case LE:
    printf("    # 令 a0 = a0 <= a1\n");
    printf("    slt a0, a1, a0\n");
    printf("    xori a0, a0, 1\n");
    return;
  case GE:
    printf("    # 令 a0 = a0 >= a1\n");
    printf("    slt a0, a0, a1\n");
    printf("    xori a0, a0, 1\n");
    return;
  default:
    break;
  }
  error("invalid expresion");
}

/**
 * @brief 生成语句节点汇编
 *
 * @param Nd 待生成节点
 */
static void genStmt(Node *node) {
  switch (node->Kind) {
    // 表达式节点
  case EXPR_STMT:
    genExpr(node->LHS);
    return;
    // 代码块节点
  case BLOCK:
    // 依次生成代码块中的语句
    for (Node *tmpNode = node->Body; tmpNode; tmpNode = tmpNode->Next) {
      genStmt(tmpNode);
    }
    return;
    // 返回节点
  case RETURN:
    // 生成返回值->a0
    genExpr(node->LHS);
    printf("    # 函数返回\n");
    printf("    j .L.return\n");
    return;
  default:
    break;
  }
  error("invalid statement");
}

/**
 * @brief 产生新的汇编生成器
 *
 * @param func 函数序列指针
 * @return Codegener* 新汇编生成器指针
 */
Codegener *newCodegener(Function *func) {
  Codegener *codegener = calloc(1, sizeof(Codegener));
  codegener->func = func;
  return codegener;
}

// 对齐到Align的整数倍
static int alignTo(int N, int Align) {
  // (0,Align]返回Align
  return (N + Align - 1) / Align * Align;
}

// 根据变量的链表计算出偏移量
static void assignLVarOffsets(Function *Prog) {
  int Offset = 0;
  // 读取所有变量
  for (Obj *Var = Prog->localObjs; Var; Var = Var->Next) {
    // 每个变量分配8字节
    Offset += 8;
    // 为每个变量赋一个偏移量，或者说是栈中地址
    Var->Offset = -Offset;
  }
  // 将栈对齐到16字节
  Prog->stackSize = alignTo(Offset, 16);
}

/**
 * @brief 汇编代码生成入口
 *
 * @param codegener 汇编生成器指针
 */
void codegen(Codegener *codegener) {
  printf("    .globl main\n");
  printf("main:\n");

  // 栈布局
  //-------------------------------//
  //  origin_temp_Stack_Top_Elemt     <-origin_sp
  //-------------------------------//
  //          origin_fp
  //-------------------------------//
  //       local_var1_addr            <-fp
  //-------------------------------//
  //       local_var2_addr
  //-------------------------------//
  //             ...
  //-------------------------------//
  //       local_varN_addr
  //-------------------------------//
  //  now_temp_Stack_Botton_Elemt     <-sp = origin_sp - 8 - StackSize
  //-------------------------------//

  // Prologue, 前言
  // 将fp压入栈中，保存fp的值
  printf("    # fp 压栈\n");
  printf("    addi sp, sp, -8\n");
  printf("    sd fp, 0(sp)\n");

  // 将sp写入fp
  printf("    # 生成变量栈区\n");
  printf("    mv fp, sp\n");
  // 计算本地变量栈空间
  assignLVarOffsets(codegener->func);
  // 偏移量为实际变量所用的栈大小
  printf("    addi sp, sp, -%d\n", codegener->func->stackSize);

  // 根节点 是一个 代码块 节点
  genStmt(codegener->func->Body);

  // Epilogue，后语
  // 恢复执行前环境

  // 将fp的值改写回sp
  printf(".L.return:\n");
  printf("    # 清理变量栈区\n");
  printf("    mv sp, fp\n");
  // 将最早fp保存的值弹栈，恢复fp。
  printf("    # 恢复 fp\n");
  printf("    ld fp, 0(sp)\n");
  printf("    # 恢复 sp\n");
  printf("    addi sp, sp, 8\n");
  // 返回
  printf("  ret\n");
}
