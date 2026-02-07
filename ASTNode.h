#pragma once

#include <vector>
#include "Token.h"
#include "Type.h"
#include "Visitor.h"

struct Visitor;

struct ASTNode {
  virtual void accept(Visitor& v) = 0;
};

struct Expression : public ASTNode {
  Token start;

  Expression(Token start) : start(start) {}

  virtual void accept(Visitor& v) = 0;
};

struct LiteralExpression : public Expression {
  Token value;

  LiteralExpression(Token value) : Expression(value), value(value) {}

  void accept(Visitor& v) override {
    v.visitLiteralExpression(*this);
  }
};

struct ArrayLiteralExpression : public Expression {
  std::vector<Expression*> elements;

  ArrayLiteralExpression(Token bracket, std::vector<Expression*> elements)
    : Expression(bracket), elements(elements) {}

  void accept(Visitor& v) override {
    v.visitArrayLiteralExpression(*this);
  }
};

struct VarExpression : public Expression {
  Token id;

  VarExpression(Token id) : Expression(id), id(id) {}

  void accept(Visitor& v) override {
    v.visitVarExpression(*this);
  }
};

struct CallExpression : public Expression {
  Expression& callee;
  std::vector<Expression*> args;

  CallExpression(Expression& callee, std::vector<Expression*> args)
    : Expression(callee.start), callee(callee), args(args) {}

  void accept(Visitor& v) override {
    v.visitCallExpression(*this);
  }
};

struct ArrayAccessExpression : public Expression {
  Expression& array;
  Expression& index;

  ArrayAccessExpression(Expression& array, Expression& index)
    : Expression(array.start), array(array), index(index) {}

  void accept(Visitor& v) override {
    v.visitArrayAccessExpression(*this);
  }
};

struct CastExpression : public Expression {
  Expression& expr;
  Type* type;

  CastExpression(Expression& expr, Type* type)
    : Expression(expr.start), expr(expr), type(type) {}

  void accept(Visitor& v) override {
    v.visitCastExpression(*this);
  }
};

struct PostfixExpression : public Expression {
  Expression& expr;
  Token op;

  PostfixExpression(Expression& expr, Token op)
    : Expression(expr.start), expr(expr), op(op) {}

  void accept(Visitor& v) override {
    v.visitPostfixExpression(*this);
  }
};

struct UnaryExpression : public Expression {
  Token op;
  Expression& expr;

  UnaryExpression(Token op, Expression& expr) : Expression(op), op(op), expr(expr) {}

  void accept(Visitor& v) override {
    v.visitUnaryExpression(*this);
  }
};

struct BinaryExpression : public Expression {
  Expression& left;
  Token op;
  Expression& right;

  BinaryExpression(Expression& left, Token op, Expression& right)
    : Expression(left.start), left(left), op(op), right(right) {}

  void accept(Visitor& v) override {
    v.visitBinaryExpression(*this);
  }
};

struct TernaryExpression : public Expression {
  Expression& condition;
  Expression& value;
  Expression& _default;

  TernaryExpression(Expression& condition, Expression& value, Expression& _default)
    : Expression(condition.start), condition(condition), value(value), _default(_default) {}

  void accept(Visitor& v) override {
    v.visitTernaryExpression(*this);
  }
};

struct AssignmentExpression : public Expression {
  Expression& l_value;
  Token op;
  Expression& value;

  AssignmentExpression(Expression& l_value, Token op, Expression& value)
    : Expression(l_value.start), l_value(l_value), op(op), value(value) {}

  void accept(Visitor& v) override {
    v.visitAssignmentExpression(*this);
  }
};

struct Statement : public ASTNode {
  virtual void accept(Visitor& v) = 0;
};

struct BlockStatement : public Statement {
  std::vector<Statement*> statements;

  BlockStatement(std::vector<Statement*> statements) : statements(statements) {}

  void accept(Visitor& v) override {
    v.visitBlockStatement(*this);
  }
};

struct ExpressionStatement : public Statement {
  Expression& expr;

  ExpressionStatement(Expression& expr) : expr(expr) {}

  void accept(Visitor& v) override {
    v.visitExpressionStatement(*this);
  }
};

struct IfStatement : public Statement {
  Expression& condition;
  Statement& thenBranch;
  Statement* elseBranch;

  IfStatement(Expression& condition, Statement& thenBranch, Statement* elseBranch)
    : condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}

  void accept(Visitor& v) override {
    v.visitIfStatement(*this);
  }
};

struct PrintStatement : public Statement {
  Expression& expr;

  PrintStatement(Expression& expr) : expr(expr) {}

  void accept(Visitor& v) override {
    v.visitPrintStatement(*this);
  }
};

struct ReturnStatement : public Statement {
  Token token;
  Expression* value;

  ReturnStatement(Token token, Expression* value) : token(token), value(value) {}

  void accept(Visitor& v) override {
    v.visitReturnStatement(*this);
  }
};

struct BreakStatement : public Statement {
  Token token;

  BreakStatement(Token token) : token(token) {}

  void accept(Visitor& v) override {
    v.visitBreakStatement(*this);
  }
};

struct ContinueStatement : public Statement {
  Token token;

  ContinueStatement(Token token) : token(token) {}

  void accept(Visitor& v) override {
    v.visitContinueStatement(*this);
  }
};

struct WhileStatement : public Statement {
  Expression& condition;
  Statement& body;

  WhileStatement(Expression& condition, Statement& body)
    : condition(condition), body(body) {}

  void accept(Visitor& v) override {
    v.visitWhileStatement(*this);
  }
};

struct Argument {
  Token id;
  Type* type;

  Argument(Token id, Type* type) : id(id), type(type) {}
};

struct FunctionDeclaration : public Statement {
  Token id;
  std::vector<Argument> args;
  Type* returnType;
  BlockStatement& body;

  FunctionDeclaration(Token id, std::vector<Argument> args, Type* returnType, BlockStatement& body)
    : id(id), args(args), returnType(returnType), body(body) {}

  void accept(Visitor& v) override {
    v.visitFunctionDeclaration(*this);
  }
};

struct VarDeclaration : public Statement {
  Token id;
  Type* type;
  Expression* initializer;

  VarDeclaration(Token id, Type* type, Expression* initializer)
    : id(id), type(type), initializer(initializer) {}

  void accept(Visitor& v) override {
    v.visitVarDeclaration(*this);
  }
};

struct ConstDeclaration : public Statement {
  Token id;
  Type* type;
  Expression& initializer;

  ConstDeclaration(Token id, Type* type, Expression& initializer)
    : id(id), type(type), initializer(initializer) {}

  void accept(Visitor& v) override {
    v.visitConstDeclaration(*this);
  }
};

struct Program : public ASTNode {
  std::vector<Statement*> statements;

  Program(std::vector<Statement*> statements) : statements(statements) {}

  void accept(Visitor& v) override {
    v.visitProgram(*this);
  } 
};