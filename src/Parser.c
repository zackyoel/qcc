#include "Compiler.h"

static Obj *LOCALOBJS;

// Rest: 分析后剩余词法单元队列指针存放位置
// Token: 要分析的词法单元

// program = "{" block  //限制程序必须被 { } 包起来
// block = stmt* "}"
// stmt = "return" expr ";" | "{" block | exprStmt
// exprStmt = expr? ";"
// expr = assign
// assign = equality ( "=" assign )?
// equality = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// shift = add ("<<" add | ">>" add)*
// add = mul ("+" mul | "-" mul)*
// mul = unary ("*" unary | "/" unary | "%" unary)*
// unary = ("+" | "-"| "*" | "&") unary | primary
// primary = "(" expr ")" | num | id
static Node *block(Token **Rest, Token *Tok);
static Node *stmt(Token **Rest, Token *Tok);
static Node *exprStmt(Token **Rest, Token *Tok);
static Node *expr(Token **Rest, Token *Tok);
static Node *assign(Token **Rest, Token *Tok);
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
static Node *newUnaryNode(NodeKind kind, Node *LHS) {
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
static Node *newBinaryNode(NodeKind kind, Node *LHS, Node *RHS) {
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
static Node *newNumNode(long value) {
  Node *node = calloc(1, sizeof(Node));
  node->Kind = NUM;
  node->Val = value;
  return node;
}

/**
 * @brief 新建变量节点
 *
 * @param obj 变量结构体指针
 * @return Node* 新变量节点指针
 */

static Node *newVarNode(Obj *obj) {
  Node *node = newNode(VAR);
  node->Var = obj;
  return node;
}

// 在链表中新增一个变量
static Obj *newLVar(char *Name) {
  Obj *var = calloc(1, sizeof(Obj));
  var->Name = Name;
  // 将变量插入头部
  var->Next = LOCALOBJS;
  LOCALOBJS = var;
  return var;
}

/**
 * @brief 从Objs队列中查找与Tok字面量相同的对象，没找到则返回 NULL
 *
 * @param Objs 变量队列
 * @param Tok  查找词法单元
 * @return Obj* 变量指针
 */
static Obj *findVar(const Token *Tok) {
  for (Obj *var = LOCALOBJS; var; var = var->Next) {
    if (strlen(var->Name) == Tok->len &&
        !strncmp(var->Name, Tok->text, Tok->len)) {
      return var;
    }
  }
  return NULL;
}

// 解析组合语句
// block = stmt* "}"
static Node *block(Token **Rest, Token *Tok) {

  // 建立 stmt 队列
  Node *headNode = calloc(1, sizeof(Node));
  Node *curNode = headNode;
  while (!equal(Tok, "}")) {
    curNode->Next = stmt(&Tok, Tok);
    curNode = curNode->Next;
  }
  // LHS 和 Body 是 union 结构，所以赋值给 LHS 同时也是赋值给 Body
  Node *node = newUnaryNode(BLOCK, headNode->Next);
  free(headNode);

  *Rest = skip(Tok, "}");
  return node;
}

// 解析语句
// stmt = "return" expr ";" | "{" block | exprStmt
static Node *stmt(Token **Rest, Token *Tok) {
  // "return" expr ";"
  if (equal(Tok, "return")) {
    Node *node = newUnaryNode(RETURN, expr(&Tok, Tok->nextTok));
    *Rest = skip(Tok, ";");
    return node;
  }
  // "{" block
  if (equal(Tok, "{")) {
    Node *node = newUnaryNode(BLOCK, block(Rest, Tok->nextTok));
    return node;
  }

  // exprStmt
  return exprStmt(Rest, Tok);
}

// 解析表达式语句
// exprStmt = expr? ";"
static Node *exprStmt(Token **Rest, Token *Tok) {
  // ";"
  if (equal(Tok, ";")) {
    *Rest = skip(Tok, ";");
    return newNode(BLOCK);
  }
  // expr? ";"
  Node *node = newUnaryNode(EXPR_STMT, expr(&Tok, Tok));
  *Rest = skip(Tok, ";");
  return node;
}

// 解析表达式
// expr = assign
static Node *expr(Token **Rest, Token *Tok) {
  // assign
  return assign(Rest, Tok);
}

// 解析赋值表达式
// assign = equality ("=" assign)?
static Node *assign(Token **Rest, Token *Tok) {
  // equality
  Node *node = equality(&Tok, Tok);

  if (equal(Tok, "=")) {
    node = newBinaryNode(ASSIGN, node, assign(&Tok, Tok->nextTok));
  }
  *Rest = Tok;
  return node;
}

// 解析相等性
// equality = relational ("==" relational | "!=" relational)*
static Node *equality(Token **Rest, Token *Tok) {
  // relational
  Node *node = relational(&Tok, Tok);

  while (true) {
    // "==" relational
    if (equal(Tok, "==")) {
      node = newBinaryNode(EQ, node, relational(&Tok, Tok->nextTok));
      continue;
    }
    // "!=" relational
    if (equal(Tok, "!=")) {
      node = newBinaryNode(NE, node, relational(&Tok, Tok->nextTok));
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
      node = newBinaryNode(LT, node, add(&Tok, Tok->nextTok));
      continue;
    }
    // "<=" add
    if (equal(Tok, "<=")) {
      node = newBinaryNode(LE, node, add(&Tok, Tok->nextTok));
      continue;
    }
    // ">" add
    if (equal(Tok, ">")) {
      node = newBinaryNode(GT, node, add(&Tok, Tok->nextTok));
      continue;
    }
    // ">=" add
    if (equal(Tok, ">=")) {
      node = newBinaryNode(GE, node, add(&Tok, Tok->nextTok));
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
      node = newBinaryNode(ADD, node, mul(&Tok, Tok->nextTok));
      continue;
    }
    // "-" mul
    if (equal(Tok, "-")) {
      node = newBinaryNode(SUB, node, mul(&Tok, Tok->nextTok));
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
      node = newBinaryNode(MUL, node, unary(&Tok, Tok->nextTok));
      continue;
    }
    // "/" unary
    if (equal(Tok, "/")) {
      node = newBinaryNode(DIV, node, unary(&Tok, Tok->nextTok));
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
    return newUnaryNode(NEG, unary(Rest, Tok->nextTok));
  }

  // "&" unary
  if (equal(Tok, "&")) {
    return newUnaryNode(ADDR, unary(Rest, Tok->nextTok));
  }

  // "*" unary
  if (equal(Tok, "*")) {
    return newUnaryNode(DEADDR, unary(Rest, Tok->nextTok));
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
    Node *node = newNumNode(Tok->intVal);
    *Rest = Tok->nextTok;
    return node;
  }

  if (Tok->kind == ID) {
    Obj *var = findVar(Tok);
    if (!var)
      var = newLVar(strndup(Tok->text, Tok->len));
    *Rest = Tok->nextTok;
    return newVarNode(var);
  }

  errorTok(Tok, "expected an expression");
  return NULL;
}

Parser *newParser(Token *tokList) {
  Parser *paser = calloc(1, sizeof(Parser));
  paser->tokList = tokList;
  return paser;
}

Function *parse(Parser *parser) {

  // "{" block
  Token *curTok = parser->tokList;
  curTok = skip(curTok, "{");

  Function *prog = calloc(1, sizeof(Function));
  prog->Body = block(&curTok, curTok);
  prog->localObjs = LOCALOBJS;

  parser->Func = prog;

  return parser->Func;
}