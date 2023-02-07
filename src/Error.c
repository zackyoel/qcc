#include "Compiler.h"

// Fmt为传入的字符串， ... 为可变参数，表示Fmt后面所有的参数

void error(char *Fmt, ...) {
  // 定义一个va_list变量
  va_list VA;
  // VA获取Fmt后面的所有参数
  va_start(VA, Fmt);
  // vfprintf可以输出va_list类型的参数
  vfprintf(stderr, Fmt, VA);
  // 在结尾加上一个换行符
  fprintf(stderr, "\n");
  // 清除VA
  va_end(VA);
  // 终止程序
  exit(1);
}

// 输出错误出现的位置，并退出
static void verrorAt(const char *context, const int col, char *Fmt, va_list VA) {
  // 先输出源信息
  fprintf(stderr, "%s\n", context);

  // 输出出错信息
  // 将字符串补齐为col位，因为是空字符串，所以填充col个空格
  fprintf(stderr, "%*s", col, "^ ");
  vfprintf(stderr, Fmt, VA);
  fprintf(stderr, "\n");
  va_end(VA);
}

// 词法分析出错
void errorAt(const Lexer* lex, char *Fmt, ...) {
  va_list VA;
  va_start(VA, Fmt);
  verrorAt(lex->curLinePtr, lex->curColNum, Fmt, VA);
  exit(1);
}

// Tok解析出错
void errorTok(Token *Tok, char *Fmt, ...) {
  va_list VA;
  va_start(VA, Fmt);
  verrorAt(Tok->context, Tok->col, Fmt, VA);
  exit(1);
}
