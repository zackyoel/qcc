#include "Compiler.h"

// Rest: 分析后剩余词法单元队列指针存放位置
// Token: 要分析的词法单元

// program = stmt*
// stmt = exprStmt
// exprStmt = expr ";"
// expr = equality
// equality = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// shift = add ("<<" add | ">>" add)*
// add = mul ("+" mul | "-" mul)*
// mul = unary ("*" unary | "/" unary | "%" unary)*
// unary = ("+" | "-"| "*" | "&") unary | primary
// primary = "(" expr ")" | num

static Node *stmt(Token **Rest, Token *Tok);
static Node *exprStmt(Token **Rest, Token *Tok);
static Node *expr(Token **Rest, Token *Tok);
static Node *equality(Token **Rest, Token *Tok);
static Node *relational(Token **Rest, Token *Tok);
static Node *add(Token **Rest, Token *Tok);
static Node *mul(Token **Rest, Token *Tok);
static Node *unary(Token **Rest, Token *Tok);
static Node *primary(Token **Rest, Token *Tok);

/**
 * @brief 创建新节点返回指针
 *
 * @param kind 节点类型
 * @return Node* 新节点指针
 */
static Node *newNode(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->Kind = kind;
  return node;
}

/**
 * @brief 新建单叉树
 *
 * @param kind 节点种类
 * @param LHS 子节点指针
 * @return Node* 新单叉树指针
 */
static Node *newUnary(NodeKind kind, Node *LHS) {
  Node *node = newNode(kind);
  node->LHS = LHS;
  return node;
}

/**
 * @brief 新建二叉树节点
 *
 * @param kind 节点种类
 * @param LHS 左子树指针
 * @param RHS 右子树指针
 * @return Node* 新节点指针
 */
static Node *newBinary(NodeKind kind, Node *LHS, Node *RHS) {
  Node *node = newNode(kind);
  node->LHS = LHS;
  node->RHS = RHS;
  return node;
}

/**
 * @brief 新建数字节点
 *
 * @param value 数字值
 * @return Node* 新节点指针
 */
static Node *newNum(long value) {
  Node *node = calloc(1, sizeof(Node));
  node->Kind = NUM;
  node->Val = value;
  return node;
}

// 解析语句
// stmt = exprStmt
static Node *stmt(Token **Rest, Token *Tok) { return exprStmt(Rest, Tok); }

// 解析表达式语句
// exprStmt = expr ";"
static Node *exprStmt(Token **Rest, Token *Tok) {
  Node *node = newUnary(EXPR_STMT, expr(&Tok, Tok));
  *Rest = skip(Tok, ";");
  return node;
}

// 解析表达式
// expr = equality
static Node *expr(Token **Rest, Token *Tok) {
  // equality
  return equality(Rest, Tok);
}

// 解析相等性
// equality = relational ("==" relational | "!=" relational)*
static Node *equality(Token **Rest, Token *Tok) {
  // relational
  Node *node = relational(&Tok, Tok);

  while (true) {
    // "==" relational
    if (equal(Tok, "==")) {
      node = newBinary(EQ, node, relational(&Tok, Tok->nextTok));
      continue;
    }
    // "!=" relational
    if (equal(Tok, "!=")) {
      node = newBinary(NE, node, relational(&Tok, Tok->nextTok));
      continue;
    }
    *Rest = Tok;
    return node;
  }
}

// 解析比较关系
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational(Token **Rest, Token *Tok) {
  // add
  Node *node = add(&Tok, Tok);
  while (true) {
    // "<" add
    if (equal(Tok, "<")) {
      node = newBinary(LT, node, add(&Tok, Tok->nextTok));
      continue;
    }
    // "<=" add
    if (equal(Tok, "<=")) {
      node = newBinary(LT, node, add(&Tok, Tok->nextTok));
      continue;
    }
    // ">" add
    if (equal(Tok, ">")) {
      node = newBinary(GT, node, add(&Tok, Tok->nextTok));
      continue;
    }
    // ">=" add
    if (equal(Tok, ">=")) {
      node = newBinary(GE, node, add(&Tok, Tok->nextTok));
      continue;
    }
    *Rest = Tok;
    return node;
  }
}

// 加减法关系
// add = mul ("+" mul | "-" mul)*
static Node *add(Token **Rest, Token *Tok) {
  // mul
  Node *node = mul(&Tok, Tok);

  while (true) {
    // "+" mul
    if (equal(Tok, "+")) {
      node = newBinary(ADD, node, mul(&Tok, Tok->nextTok));
      continue;
    }
    // "-" mul
    if (equal(Tok, "-")) {
      node = newBinary(SUB, node, mul(&Tok, Tok->nextTok));
      continue;
    }
    *Rest = Tok;
    return node;
  }
}

// 乘除取余关系
// mul = unary ("*" unary | "/" unary )
static Node *mul(Token **Rest, Token *Tok) {
  // unary
  Node *node = unary(&Tok, Tok);

  while (true) {
    // "*" unary
    if (equal(Tok, "*")) {
      node = newBinary(MUL, node, unary(&Tok, Tok->nextTok));
      continue;
    }
    // "/" unary
    if (equal(Tok, "/")) {
      node = newBinary(DIV, node, unary(&Tok, Tok->nextTok));
      continue;
    }
    *Rest = Tok;
    return node;
  }
}

// 解析一元运算符
// unary = ("+" | "-" | "&" | "*")* unary | primary
static Node *unary(Token **Rest, Token *Tok) {

  // "+" unary
  if (equal(Tok, "+")) {
    return unary(Rest, Tok->nextTok);
  }

  // "-" unary
  if (equal(Tok, "-")) {
    return newUnary(NEG, unary(Rest, Tok->nextTok));
  }

  // "&" unary
  if (equal(Tok, "&")) {
    return newUnary(ADDR, unary(Rest, Tok->nextTok));
  }

  // "*" unary
  if (equal(Tok, "*")) {
    return newUnary(DEADDR, unary(Rest, Tok->nextTok));
  }

  // primary
  return primary(Rest, Tok);
}

// primary = "(" expr ")" |num
static Node *primary(Token **Rest, Token *Tok) {
  if (equal(Tok, "(")) {
    Node *node = expr(&Tok, Tok->nextTok);
    *Rest = skip(Tok, ")");
    return node;
  }

  if (Tok->kind == VAL_INTEGER) {
    Node *node = newNum(Tok->intVal);
    *Rest = Tok->nextTok;
    return node;
  }

  errorTok(Tok, "expected an expression");
  return NULL;
}

Parser *newParser(Token *tokList) {
  Parser *paser = calloc(1, sizeof(Parser));
  paser->tokList = tokList;
  return paser;
}

Node *parse(Parser *parser) {
  Token *Tok = parser->tokList;
  parser->astRoot = calloc(1, sizeof(Node));

  for (Node *curNode = parser->astRoot; Tok->kind; curNode = curNode->next) {
    curNode->next = stmt(&Tok, Tok);
  }
  
  parser->astRoot = parser->astRoot->next;
  return parser->astRoot->next;
}