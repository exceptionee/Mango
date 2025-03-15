#pragma once

#include <any>

struct ASTNode;
struct Program;
struct Statement;
struct IfStatement;
struct BlockStatement;
struct ExpressionStatement;
struct PrintStatement;
struct WhileStatement;
struct VarDeclarationStatement;
struct ConstDeclarationStatement;
struct Expression;
struct LiteralExpression;
struct ArrayLiteralExpression;
struct VarExpression;
struct ArrayAccessExpression;
struct PostfixExpression;
struct UnaryExpression;
struct BinaryExpression;
struct TernaryExpression;
struct AssignmentExpression;

struct Visitor {
  virtual std::any visit(ASTNode& e) = 0;
  virtual std::any visitProgram(Program& p) = 0;
  virtual std::any visitIfStatement(IfStatement& s) = 0;
  virtual std::any visitBlockStatement(BlockStatement& s) = 0;
  virtual std::any visitExpressionStatement(ExpressionStatement& s) = 0;
  virtual std::any visitPrintStatement(PrintStatement& s) = 0;
  virtual std::any visitWhileStatement(WhileStatement& s) = 0;
  virtual std::any visitVarDeclarationStatement(VarDeclarationStatement& s) = 0;
  virtual std::any visitConstDeclarationStatement(ConstDeclarationStatement& s) = 0;
  virtual std::any visitLiteralExpression(LiteralExpression& e) = 0;
  virtual std::any visitArrayLiteralExpression(ArrayLiteralExpression& e) = 0;
  virtual std::any visitVarExpression(VarExpression& e) = 0;
  virtual std::any visitArrayAccessExpression(ArrayAccessExpression& e) = 0;
  virtual std::any visitPostfixExpression(PostfixExpression& e) = 0;
  virtual std::any visitUnaryExpression(UnaryExpression& e) = 0;
  virtual std::any visitBinaryExpression(BinaryExpression& e) = 0;
  virtual std::any visitTernaryExpression(TernaryExpression& e) = 0;
  virtual std::any visitAssignmentExpression(AssignmentExpression& e) = 0;
};