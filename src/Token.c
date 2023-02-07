#include "Compiler.h"

Token *newToken(TokenKind kind, const char *text, const char *fpath,
                const char *context, int row, int col) {
  //分配1个Token空间
  Token *token = calloc(1, sizeof(Token));

  //初始化变量
  token->kind = kind;
  token->text = text;
  token->fpath = fpath;
  token->context = context;
  token->row = row;
  token->col = col;

  //返回新生成token
  return token;
}

bool equal(const Token *Tok, const char *Str) {
  // memcmp(s1, s2, n)比较s1和s2的前n位，s2长度应该大于n，
  // 比较按照字典序，相同则返回0
  // 确保长度相同
  return memcmp(Tok->text, Str, Tok->len) == 0 && Str[Tok->len] == '\0';
}

Token *skip(Token *Tok, char *Str) {
  if (!equal(Tok, Str)) {
    errorTok(Tok, "expect '%s'", Str);
  }
  return Tok->nextTok;
}