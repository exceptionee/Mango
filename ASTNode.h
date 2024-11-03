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

struct Expression : ASTNode {
  Token start;

  Expression(Token& start) : start(start) {}

  virtual std::any accept(Visitor& v) = 0;
};

struct LiteralExpression : Expression {
  Token value;

  LiteralExpression(Token value) : Expression(value), value(value) {}

  std::any accept(Visitor& v) override {
    return v.visitLiteralExpression(*this);
  }
};

struct ArrayLiteralExpression : Expression {
  std::vector<Expression*> elements;

  ArrayLiteralExpression(Token lb, std::vector<Expression*>& elements)
    : Expression(lb), elements(elements) {}

  std::any accept(Visitor& v) override {
    return v.visitArrayLiteralExpression(*this);
  }
};

struct VarExpression : Expression {
  Token id;

  VarExpression(Token id) : Expression(id), id(id) {}

  std::any accept(Visitor& v) override {
    return v.visitVarExpression(*this);
  }
};

struct ArrayAccessExpression : Expression {
  Expression& array;
  Expression& index;

  ArrayAccessExpression(Expression& array, Expression& index)
    : Expression(array.start), array(array), index(index) {}

  std::any accept(Visitor& v) override {
    return v.visitArrayAccessExpression(*this);
  }
};

struct PostfixExpression : Expression {
  Expression& expr;
  Token op;

  PostfixExpression(Expression& expr, Token op)
    : Expression(expr.start), expr(expr), op(op) {}

  std::any accept(Visitor& v) override {
    return v.visitPostfixExpression(*this);
  }
};

struct UnaryExpression : Expression {
  Token op;
  Expression& expr;

  UnaryExpression(Token op, Expression& expr) : Expression(op), op(op), expr(expr) {}

  std::any accept(Visitor& v) override {
    return v.visitUnaryExpression(*this);
  }
};

struct BinaryExpression : Expression {
  Expression& left;
  Token op;
  Expression& right;

  BinaryExpression(Expression& left, Token op, Expression& right)
    : Expression(left.start), left(left), op(op), right(right) {}

  std::any accept(Visitor& v) override {
    return v.visitBinaryExpression(*this);
  }
};

struct TernaryExpression : Expression {
  Expression& condition;
  Expression& value;
  Expression& _default;

  TernaryExpression(Expression& condition, Expression& value, Expression& _default)
    : Expression(condition.start), condition(condition), value(value), _default(_default) {}

  std::any accept(Visitor& v) override {
    return v.visitTernaryExpression(*this);
  }
};

struct AssignmentExpression : Expression {
  Expression& l_value;
  Token op;
  Expression& value;

  AssignmentExpression(Expression& l_value, Token op, Expression& value)
    : Expression(l_value.start), l_value(l_value), op(op), value(value) {}

  std::any accept(Visitor& v) override {
    return v.visitAssignmentExpression(*this);
  }
};

struct Statement : ASTNode {
  virtual std::any accept(Visitor& v) = 0;
};

struct BlockStatement : Statement {
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

struct PrintStatement : public Statement {
  Expression& expr;

  PrintStatement(Expression& expr) : expr(expr) {}

  std::any accept(Visitor& v) {
    return v.visitPrintStatement(*this);
  }
};

struct VarDeclarationStatement : public Statement {
  Token id;
  Type* type;
  Expression* initializer;

  VarDeclarationStatement(Token id, Type* type, Expression* initializer)
    : id(id), type(type), initializer(initializer) {}

  std::any accept(Visitor& v) {
    return v.visitVarDeclarationStatement(*this);
  }
};

struct ConstDeclarationStatement : public Statement {
  Token id;
  Type* type;
  Expression& initializer;

  ConstDeclarationStatement(Token id, Type* type, Expression& initializer)
    : id(id), type(type), initializer(initializer) {}

  std::any accept(Visitor& v) {
    return v.visitConstDeclarationStatement(*this);
  }
};

struct Program : ASTNode {
  std::vector<Statement*> statements;

  Program(std::vector<Statement*> statements) : statements(statements) {}

  std::any accept(Visitor& v) {
    return v.visitProgram(*this);
  } 
};