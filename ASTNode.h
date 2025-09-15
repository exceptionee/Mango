#pragma once

#include <any>
#include <vector>
#include "Token.h"
#include "Type.h"
#include "Visitor.h"

struct Visitor;

struct ASTNode {
  virtual std::any accept(Visitor& v) = 0;
};

struct Expression : public ASTNode {
  Token start;

  Expression(Token& start) : start(start) {}

  virtual std::any accept(Visitor& v) = 0;
};

struct LiteralExpression : public Expression {
  Token value;

  LiteralExpression(Token value) : Expression(value), value(value) {}

  std::any accept(Visitor& v) override {
    return v.visitLiteralExpression(*this);
  }
};

struct ArrayLiteralExpression : public Expression {
  std::vector<Expression*> elements;

  ArrayLiteralExpression(Token bracket, std::vector<Expression*>& elements)
    : Expression(bracket), elements(elements) {}

  std::any accept(Visitor& v) override {
    return v.visitArrayLiteralExpression(*this);
  }
};

struct VarExpression : public Expression {
  Token id;

  VarExpression(Token id) : Expression(id), id(id) {}

  std::any accept(Visitor& v) override {
    return v.visitVarExpression(*this);
  }
};

struct CallExpression : public Expression {
  Expression& callee;
  std::vector<Expression*> args;
  Token closeParen;

  CallExpression(Expression& callee, std::vector<Expression*> args, Token closeParen)
    : Expression(callee.start), callee(callee), args(args), closeParen(closeParen) {}

  std::any accept(Visitor& v) override {
    return v.visitCallExpression(*this);
  }
};

struct ArrayAccessExpression : public Expression {
  Expression& array;
  Expression& index;

  ArrayAccessExpression(Expression& array, Expression& index)
    : Expression(array.start), array(array), index(index) {}

  std::any accept(Visitor& v) override {
    return v.visitArrayAccessExpression(*this);
  }
};

struct CastExpression : public Expression {
  Expression& expr;
  Type* type;

  CastExpression(Expression& expr, Type* type)
    : Expression(expr.start), expr(expr), type(type) {}

  std::any accept(Visitor& v) override {
    return v.visitCastExpression(*this);
  }
};

struct PostfixExpression : public Expression {
  Expression& expr;
  Token op;

  PostfixExpression(Expression& expr, Token op)
    : Expression(expr.start), expr(expr), op(op) {}

  std::any accept(Visitor& v) override {
    return v.visitPostfixExpression(*this);
  }
};

struct UnaryExpression : public Expression {
  Token op;
  Expression& expr;

  UnaryExpression(Token op, Expression& expr) : Expression(op), op(op), expr(expr) {}

  std::any accept(Visitor& v) override {
    return v.visitUnaryExpression(*this);
  }
};

struct BinaryExpression : public Expression {
  Expression& left;
  Token op;
  Expression& right;

  BinaryExpression(Expression& left, Token op, Expression& right)
    : Expression(left.start), left(left), op(op), right(right) {}

  std::any accept(Visitor& v) override {
    return v.visitBinaryExpression(*this);
  }
};

struct TernaryExpression : public Expression {
  Expression& condition;
  Expression& value;
  Expression& _default;

  TernaryExpression(Expression& condition, Expression& value, Expression& _default)
    : Expression(condition.start), condition(condition), value(value), _default(_default) {}

  std::any accept(Visitor& v) override {
    return v.visitTernaryExpression(*this);
  }
};

struct AssignmentExpression : public Expression {
  Expression& l_value;
  Token op;
  Expression& value;

  AssignmentExpression(Expression& l_value, Token op, Expression& value)
    : Expression(l_value.start), l_value(l_value), op(op), value(value) {}

  std::any accept(Visitor& v) override {
    return v.visitAssignmentExpression(*this);
  }
};

struct Statement : public ASTNode {
  virtual std::any accept(Visitor& v) = 0;
};

struct BlockStatement : public Statement {
  std::vector<Statement*> statements;

  BlockStatement(std::vector<Statement*> statements) : statements(statements) {}

  std::any accept(Visitor& v) {
    return v.visitBlockStatement(*this);
  }
};

struct ExpressionStatement : public Statement {
  Expression& expr;

  ExpressionStatement(Expression& expr) : expr(expr) {}

  std::any accept(Visitor& v) {
    return v.visitExpressionStatement(*this);
  }
};

struct IfStatement : public Statement {
  Expression& condition;
  Statement& thenBranch;
  Statement* elseBranch;

  IfStatement(Expression& condition, Statement& thenBranch, Statement* elseBranch)
    : condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}

  std::any accept(Visitor& v) {
    return v.visitIfStatement(*this);
  }
};

struct PrintStatement : public Statement {
  Expression& expr;

  PrintStatement(Expression& expr) : expr(expr) {}

  std::any accept(Visitor& v) {
    return v.visitPrintStatement(*this);
  }
};

struct ReturnStatement : public Statement {
  Expression* value;

  ReturnStatement(Expression* value)
    : value(value) {}

  std::any accept(Visitor& v) {
    return v.visitReturnStatement(*this);
  }
};

struct WhileStatement : public Statement {
  Expression& condition;
  Statement& body;

  WhileStatement(Expression& condition, Statement& body)
    : condition(condition), body(body) {}

  std::any accept(Visitor& v) {
    return v.visitWhileStatement(*this);
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

  std::any accept(Visitor& v) {
    return v.visitFunctionDeclaration(*this);
  }
};

struct VarDeclaration : public Statement {
  Token id;
  Type* type;
  Expression* initializer;

  VarDeclaration(Token id, Type* type, Expression* initializer)
    : id(id), type(type), initializer(initializer) {}

  std::any accept(Visitor& v) {
    return v.visitVarDeclaration(*this);
  }
};

struct ConstDeclaration : public Statement {
  Token id;
  Type* type;
  Expression& initializer;

  ConstDeclaration(Token id, Type* type, Expression& initializer)
    : id(id), type(type), initializer(initializer) {}

  std::any accept(Visitor& v) {
    return v.visitConstDeclaration(*this);
  }
};

struct Program : public ASTNode {
  std::vector<Statement*> statements;

  Program(std::vector<Statement*> statements) : statements(statements) {}

  std::any accept(Visitor& v) {
    return v.visitProgram(*this);
  } 
};