#pragma once

#include "ASTNode.h"

struct ASTNode;
struct Program;
struct BlockStatement;
struct ExpressionStatement;
struct IfStatement;
struct PrintStatement;
struct ReturnStatement;
struct WhileStatement;
struct FunctionDeclaration;
struct VarDeclaration;
struct ConstDeclaration;
struct LiteralExpression;
struct ArrayLiteralExpression;
struct VarExpression;
struct CallExpression;
struct ArrayAccessExpression;
struct CastExpression;
struct PostfixExpression;
struct UnaryExpression;
struct BinaryExpression;
struct TernaryExpression;
struct AssignmentExpression;

struct Visitor {
  virtual void visit(ASTNode& e) = 0;
  virtual void visitProgram(Program& p) = 0;
  virtual void visitBlockStatement(BlockStatement& s) = 0;
  virtual void visitExpressionStatement(ExpressionStatement& s) = 0;
  virtual void visitIfStatement(IfStatement& s) = 0;
  virtual void visitPrintStatement(PrintStatement& s) = 0;
  virtual void visitReturnStatement(ReturnStatement& s) = 0;
  virtual void visitWhileStatement(WhileStatement& s) = 0;
  virtual void visitFunctionDeclaration(FunctionDeclaration& s) = 0;
  virtual void visitVarDeclaration(VarDeclaration& s) = 0;
  virtual void visitConstDeclaration(ConstDeclaration& s) = 0;
  virtual void visitLiteralExpression(LiteralExpression& e) = 0;
  virtual void visitArrayLiteralExpression(ArrayLiteralExpression& e) = 0;
  virtual void visitVarExpression(VarExpression& e) = 0;
  virtual void visitCallExpression(CallExpression& e) = 0;
  virtual void visitArrayAccessExpression(ArrayAccessExpression& e) = 0;
  virtual void visitCastExpression(CastExpression& e) = 0;
  virtual void visitPostfixExpression(PostfixExpression& e) = 0;
  virtual void visitUnaryExpression(UnaryExpression& e) = 0;
  virtual void visitBinaryExpression(BinaryExpression& e) = 0;
  virtual void visitTernaryExpression(TernaryExpression& e) = 0;
  virtual void visitAssignmentExpression(AssignmentExpression& e) = 0;
};