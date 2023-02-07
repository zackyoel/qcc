#ifndef COMPILER_H
#define COMPILER_H

// 使用POSIX.1标准
// 使用了strndup函数
#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/************************Token************************/

//词法单元种类
typedef enum {
  EOF_FLAG, // EOF

  // 符号
  PUNCT, // 各类标点符号

  // 关键字
  KEYWORD, // 各种关键词

  // 值
  VAL_CHAR,    //字符值
  VAL_INTEGER, //整形值
  VAL_FLOAT,   //浮点值
  VAL_STRING,  //字符串

  // 标识符, [a-zA-Z_][a-zA-Z0-9_]*
  ID, // identifier
} TokenKind;

//词法单元结构体
typedef struct Token Token;
struct Token {
  const char *text; // 文本指针
  unsigned int len; // 文本长度
  TokenKind kind;   // 词法单元种类

  const char *fpath;   // 所在文件地址
  const char *context; // 上下文
  unsigned int row;    // 行位置
  unsigned int col;    // 列位置

  union {
    char charVal;    // 字符值
    long intVal;     // 整形数值
    double floatVal; // 浮点数值
    char *strPtr;    // 字符串首指针
  };

  Token *nextTok; // 下一个词法单元
};

/**
 * @brief 生成并返回一个新的Token指针
 *
 * @param kind Token类型
 * @param text Token文本起始指针
 * @param fpath Token所在文件
 * @param ftext Token所在文件文本
 * @param line  Token所在行
 * @param pos   Token所在列
 * @return Token* 新生成的Token指针
 */
Token *newToken(TokenKind kind, const char *text, const char *fpath,
                const char *context, int row, int col);

/**
 * @brief 比较Token->text与Str内容
 *
 * @param Tok 待比较词法单元
 * @param Str 待比较字符串
 * @return true 内容相同
 * @return false 内容不同
 */
bool equal(const Token *Tok, const char *Str);

/**
 * @brief 跳过文本内容与Str相同的Token，相同则返回下一个Token指针，否则报错
 *
 * @param Buf 上下文
 * @param Tok 当前词法单元
 * @param Str 匹配字符串
 * @return Token* 下一个词法单元指针
 */
Token *skip(Token *Tok, char *Str);

/**
 * @brief 检查该词法单元是否为KEYWORD，并转换
 *
 * @param Tok
 */
void convert(Token *Tok);

/************************Lexer************************/

// 词法分析器结构体
typedef struct Lexer Lexer;
struct Lexer {
  Token *tokListHead; // 词法单元队列

  const char *fText; // 文本指针
  const char *fPath; // 打开路径

  const char *curReadPtr; // 当前读取指针
  const char *curLinePtr; // 当前行首字符指针
  int curRowNum;          // 当前读取行位置
  int curColNum;          // 当前读取列位置
};

/**
 * @brief 生成并初始化一个词法律分析器
 *
 * @param fpath 等编译文件地址
 * @return Lexer* 新生成编译器地址
 */
Lexer *newLexer(const char *fpath);

/**
 * @brief 为词法分析器产生词法单元序列并返回
 *
 * @param lexer 待生成词法单元序列的词法分析器
 * @return Token* 词法单元序列头节点
 */
Token *analysis(Lexer *lexer);

/************************Parser************************/

// AST节点种类
typedef enum {
  BLOCK,     // 代码块
  EXPR_STMT, // expr ";"
  NUM,       // 常数
  VAR,       // 对象变量

  // 关键字节点

  // 分支相关
  IF,      // if
  ELSE,    // else
  GOTO,    // goto
  SWITCH,  // switch
  CASE,    // case
  DEFAULT, // default

  // 循环相关
  FOR,      // for
  DO,       // do
  WHILE,    // while
  BREAK,    // break
  CONTINUE, // continue

  RETURN, // return
  SIZEOF, // sizeof

  // 基本数据类型
  VOID,   // void
  CHAR,   // char
  SHORT,  // short
  INT,    // int
  LONG,   // long
  FLOAT,  // float
  DOUBLE, // double

  // 创建新类型
  UNION,  // union
  ENUM,   // enum
  STRUCT, // struct
  TYPDEF, // typdef

  // 附加声明
  AUTO,     // auto
  EXTERN,   // extern
  CONST,    // const
  STATIC,   // static
  SIGNED,   // signed
  UNSIGNED, // unsigned
  REGISTER, // register
  VOLATILE, // volatile

  //运算符节点 优先级：低->高
  COMMA, // , 逗号

  ASSIGN,     // =  赋值
  ADD_ASSIGN, // += 加赋值
  SUB_ASSIGN, // -= 减赋值
  MUL_ASSIGN, // *= 乘赋值
  DIV_ASSIGN, // /= 除赋值
  MOD_ASSIGN, // %= 取余赋值
  SHL_ASSIGN, // <<=  左移赋值
  SHR_ASSIGN, // >>=  右移赋值
  AND_ASSIGN, // &=   按位与赋值
  OR_ASSIGN,  // |=   按位或赋值
  XOR_ASSIGN, // ^=   按位异或赋值

  CONDITION, // ?: 三元条件运算符

  LOGIC_OR, // || 逻辑或

  LOGIC_AND, // && 逻辑与

  OR, // | 按位或

  XOR, // ^ 按位异或

  AND, // & 按位与

  EQ, // == 等于
  NE, // != 不等

  LT, // <  小于
  GT, // >  大于
  LE, // <= 小于等于
  GE, // >= 大于等于

  SHL, // << 左移
  SHR, // >> 右移

  ADD, // + 加号
  SUB, // - 减号

  MUL, // * 乘法
  DIV, // / 除号
  MOD, // % 百分号

  NEG,       // -  负号
  NOT,       // ~  按位反
  INC,       // ++ 自增
  DEC,       // -- 自减
  DEADDR,    // *  地址解析
  ADDR,      // &  取地址
  LOGIC_NOT, // !  逻辑非

  INDEX,       // [] 数组下标
  PARANTHESES, // () 括号
  ARROW,       // -> 箭头
  DOT,         // .  点

} NodeKind;

// AST语法树节点结构体
typedef struct Node Node;
// 对象结构体
typedef struct Obj Obj;
// 函数结构体
typedef struct Function Function;

struct Obj {
  Obj *Next;   // 下个对象名
  char *Name;  // 对象名
  long Offset; // 相对 fp 的偏移量
};

struct Function {
  Node *Body;     // 函数体
  Obj *localObjs; // 函数局部变量
  int stackSize;  // 函数栈大小
};

struct Node {
  NodeKind Kind; // 节点种类
  union {
    int Val;   // 值
    Node *LHS; // 左子树
    Obj *Var;  //变量
    Node *Body;
  };
  union {
    Node *RHS;  // 右子树
    Node *Next; // 右兄弟
  };
};

//语法分析器结构体
typedef struct Parser Parser;
struct Parser {
  Token *tokList;
  Function *Func;
};

/**
 * @brief  从词法单元序列生成一个语法分析树

 * @param  toklist 词法单元序列头节点
 * @return Parser* 新生成的语法分析器指针
 */
Parser *newParser(Token *tokList);

/**
 * @brief 为语法分析器进行语法分析，返回语法分析树
 *
 * @param parser 待分析语法分析器
 * @return Function* 函数结构体指针
 */
Function *parse(Parser *parser);

/************************Codegener************************/
typedef struct Codegener Codegener;

struct Codegener {
  int stackDepth; //压栈深度
  Function *func; //语法分析树根节点
};
Codegener *newCodegener(Function *func);
void codegen(Codegener *codegener);

/************************Error************************/
//基本错误处理
void error(char *Fmt, ...);
void errorAt(const Lexer *lex, char *Fmt, ...);
void errorTok(Token *Tok, char *Fmt, ...);

#endif