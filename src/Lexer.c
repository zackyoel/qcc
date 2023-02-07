#include "Compiler.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// static为仅限本文件内可用，类似于C++类的private声明

/**
 * @brief 从文件地址获取文件文本
 *
 * @param fpath 指定文件地址 若为-则从标准输入读取
 * @return char* 文件文本
 */
static char *readFile(const char *fpath) {

  // 读取文件内容
  FILE *fptr;
  if (strcmp(fpath, "-") == 0) {
    // 如果文件名是"-"，那么就从输入中读取
    fptr = stdin;
  } else {
    fptr = fopen(fpath, "r");
    if (!fptr) {
      // errno为系统最后一次的错误代码
      // strerror以字符串的形式输出错误代码
      error("cannot open %s: %s", fpath, strerror(errno));
    }
  }

  // 返回的字符串
  char *fText;
  // 返回字符串长度
  size_t fTextLen;

  // 将fText映射到文件流Out中
  // 对Out文件的读写操作将同步到fText中，同时长度也将同步到fTextLen中
  FILE *Out = open_memstream(&fText, &fTextLen);

  // 读取整个文件
  while (true) {

    // 数组指针BufTemp，数组元素大小1，数组元素个数4096，文件流指针
    char BufTmp[4096];

    // fread从文件流中读取数据到数组中
    int N = fread(BufTmp, 1, sizeof(BufTmp), fptr);

    // 检查是否读取出新的字符串
    if (N == 0)
      break;

    // 向Out文件流中写入字符串
    // 相当于向fText中写入字符串
    fwrite(BufTmp, 1, N, Out);
  }

  // 读取完成关闭文件
  if (fptr != stdin) {
    fclose(fptr);
  }

  // 刷新流缓冲区，确保内容写入
  fflush(Out);

  // 确保最后一行'\n'结尾
  if (fTextLen == 0 || fText[fTextLen - 1] != '\n') {
    fputc('\n', Out);
  }

  // 确保fText文件至少有结尾
  fputc('\0', Out);

  // 返回指针待编译字符串指针
  return fText;
}

/**
 * @brief 检查Str是否由SubStr开头
 *
 * @param Str 待检测字符串
 * @param SubStr 匹配字符串
 * @return true 相同
 * @return false 不同
 */
static bool startWith(const char *Str, const char *SubStr) {
  // 比较s1和s2的前n个字符是否相同
  return !strncmp(Str, SubStr, strlen(SubStr));
}

/**
 * @brief 获取C语言操作符长度
 *
 * @param Ptr 待检测字符串起始
 * @return unsigned int 操作符长度，若不是返回0
 */
static unsigned int punctLen(const char *str) {
  if (startWith(str, "==") || startWith(str, "!=") || startWith(str, "<=") ||
      startWith(str, ">=")) {
    return 2;
  }
  return ispunct(*str) ? 1 : 0;
}

/**
 * @brief 判断字符串首字符符合变量要求 [a-zA-Z_]
 *
 * @param str 待检测字符串
 * @return true 可以为变量首字符
 * @return false 不能为变量首字符
 */
static bool isVarStart(const char *str) {
  if (isalpha(*str) || startWith(str, "_"))
    return true;
  return false;
}

/**
 * @brief 获取变量名长度 [a-zA-Z_][a-zA-Z0-9_]*
 *
 * @param str 待检测字符串
 * @return unsigned int 能形成的最长字符串长度
 */
static unsigned int varLen(const char *str) {
  if (isVarStart(str)) {
    int len = 1;
    const char *tempChar = str;
    while (isalnum(*tempChar) || startWith(tempChar, "_")) {
      len++;
    }
    return len;
  }
  return 0;
}

/**
 * @brief 检查字符串是否为16进制开头标志 [ "0x" || "0X" ]
 *
 * @param str 待检测字符串
 */
static bool isHexStart(const char *str) {
  //检查开头是否符合
  if (startWith(str, "0x") || startWith(str, "0X")) {
    return true;
  }
  return false;
}

/**
 * @brief 检查字符串是否为8进制数字 0[0-9a-fA-F]+
 *
 * @param str 待检测字符串
 */
static bool isOctStart(const char *str) {
  if (startWith(str, "0")) {
    return true;
  }
  return false;
}

Lexer *newLexer(const char *fpath) {

  // 动态申请词法分析器空间
  Lexer *lexer = calloc(1, sizeof(Lexer));

  // 初始化词法分析器参数
  lexer->fText = readFile(fpath);
  lexer->fPath = fpath;
  lexer->tokListHead = calloc(1, sizeof(Token));
  lexer->curRowNum = 1;
  lexer->curColNum = 1;

  //返回词法分析器指针
  return lexer;
}

Token *analysis(Lexer *lexer) {
  //初始化读取位置
  lexer->curReadPtr = lexer->fText;
  lexer->curLinePtr = lexer->fText;

  //当前token指针
  Token *CurTok = lexer->tokListHead;

  //循环读取lexer->curReadPtr
  while (*(lexer->curReadPtr)) {

    //空白符处理
    if (isspace(*(lexer->curReadPtr))) {
      //回车符处理
      if (*(lexer->curReadPtr) == '\n') {
        // 读取行位置+1，列位置重置
        ++(lexer->curRowNum);
        lexer->curColNum = 0;
        // 更新读取行首指针
        lexer->curLinePtr = lexer->curReadPtr + 1;
      }
      //更新读取位置
      ++(lexer->curReadPtr);
      ++(lexer->curColNum);
      continue;
    }

    //解析数字 [0-9]*
    if (isdigit(*(lexer->curReadPtr))) {
      //尾插法生成token队列
      CurTok->nextTok =
          newToken(VAL_INTEGER, lexer->curReadPtr, lexer->fPath,
                   lexer->curLinePtr, lexer->curRowNum, lexer->curColNum);
      CurTok = CurTok->nextTok;

      //获取数字值，读取位置移动
      if (isHexStart(lexer->curReadPtr)) {
        CurTok->intVal =
            strtoul(lexer->curReadPtr, (char **)&(lexer->curReadPtr), 16);
      } else if (isOctStart(lexer->curReadPtr)) {
        CurTok->intVal =
            strtoul(lexer->curReadPtr, (char **)&(lexer->curReadPtr), 8);
      } else {
        CurTok->intVal =
            strtoul(lexer->curReadPtr, (char **)&(lexer->curReadPtr), 10);
      }

      //获取token文本长度
      CurTok->len = lexer->curReadPtr - CurTok->text;
      //更新词法分析器读取位置
      lexer->curColNum += CurTok->len;
      continue;
    }

    // 解析变量名 [a-zA-Z_][a-zA-Z0-9_]*
    unsigned int VarLen = varLen(lexer->curReadPtr);
    if (VarLen) {
      CurTok->nextTok =
          newToken(ID, lexer->curReadPtr, lexer->fPath, lexer->curLinePtr,
                   lexer->curRowNum, lexer->curColNum);
      CurTok->len = VarLen;
      //更新词法分析器读取位置
      lexer->curColNum += CurTok->len;
      lexer->curReadPtr += CurTok->len;
      continue;
    }

    //解析各类操作符
    unsigned int PunctLen = punctLen(lexer->curReadPtr);
    if (PunctLen) {
      //尾插法生成token队列
      CurTok->nextTok =
          newToken(PUNCT, lexer->curReadPtr, lexer->fPath, lexer->curLinePtr,
                   lexer->curRowNum, lexer->curColNum);
      CurTok = CurTok->nextTok;
      CurTok->len = PunctLen;
      //更新词法分析器读取位置
      lexer->curColNum += CurTok->len;
      lexer->curReadPtr += CurTok->len;
      continue;
    }

    //目前无法处理的字符
    errorAt(lexer, "invalid token");
  }

  //插入最后一个文本终结token EOF_FLAG
  CurTok->nextTok =
      newToken(EOF_FLAG, lexer->curReadPtr, lexer->fText, lexer->curLinePtr,
               lexer->curRowNum, lexer->curColNum);

  // lexer->tokListHead只是一个头节点，有效信息为其nextToken
  return lexer->tokListHead->nextTok;
}