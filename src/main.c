#include "Compiler.h"

int main(int args, char **argv) {

  // 检查传入参数是否为2个，arg[0]为程序名称，argv[1]为传入的第一个参数
  // fprintf，格式化文件输出，向文件流stream中写入格式化字符串
  // stderr，异常文件，向屏幕输出异常信息
  // %s，字符串通配符号
  if (args != 2) {
    fprintf(stderr, "%s: invalid number of aruguments\n", argv[0]);
    return 1;
  }

  //词法分析
  Lexer *lexer = newLexer(argv[1]);
  Token *toklist = analysis(lexer);

  //语法分析
  Parser *parser = newParser(toklist);
  Function *func = parse(parser);

  //目标代码生成
  Codegener *codegener = newCodegener(func);
  codegen(codegener);

  return 0;
}