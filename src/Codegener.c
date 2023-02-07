#include "Compiler.h"

static unsigned int Depth;

static void push(void) {
  printf("    # 将 a0 中的值压入内存栈顶\n");
  printf("    addi sp, sp, -8\n");
  printf("    sd a0, 0(sp)\n");
  Depth++;
}

static void pop(char *reg) {
  printf("    # 将 栈顶 栈入 %s 寄存器\n", reg);
  printf("    ld %s, 0(sp)\n", reg);
  printf("    addi sp, sp, 8\n");
  Depth--;
}

static void genExpr(Node *node) {
  switch (node->Kind) {
  case EXPR_STMT:
    genExpr(node->LHS);
    if (node->next) {
      genExpr(node->next);
    }
    return;
  case NUM:
    printf("    # 将常数值写入 a0\n");
    printf("    li a0, %d\n", node->Val);
    return;
  case NEG:
    genExpr(node->LHS);
    printf("    # a0 中的值取反放入 a0\n");
    printf("    neg a0, a0\n");
    return;
  default:
    break;
  }

  genExpr(node->RHS);
  push();
  genExpr(node->LHS);
  pop("a1");

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
      printf("    # 比较 a0 是否等于 0结果放入 a0\n");
      printf("    seqz a0, a0\n");
    } else {
      printf("    # 比较 a0 是否不等于 0 结果放入 a0\n");
      printf("    snez a0, a0\n");
    }
    return;
  case LT:
    printf("    # 比较 a0 小于 a1 结果放入 a0\n");
    printf("    slt a0, a0, a1\n");
    return;
  case GT:
    printf("    # 比较 a0 大于 a1 结果放入 a0\n");
    printf("    slt a0, a1, a0\n");
    return;
  case LE:
    printf("    # 比较 a0 小于等于 a1 结果放入 a0\n");
    printf("    slt a0, a1, a0\n");
    printf("    xori a0, a0, 1\n");
  case GE:
    printf("    # 比较 a0 大于等于 a1 结果放入 a0\n");
    printf("    slt a0, a0, a1\n");
    printf("    xori a0, a0, 1\n");
  default:
    break;
  }
  error("invalid expresion");
}

Codegener *newCodegener(Node *astRoot) {
  Codegener *codegener = calloc(1, sizeof(Codegener));
  codegener->astRoot = astRoot;
  return codegener;
}

void Codegen(Codegener *codegener) {
  printf("    .globl main\n");
  printf("main:\n");
  genExpr(codegener->astRoot);
  printf("    ret\n");
}