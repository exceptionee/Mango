#pragma once

#include <any>
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
struct ArrayAccessExpression;
struct PostfixExpression;
struct UnaryExpression;
struct BinaryExpression;
struct TernaryExpression;
struct AssignmentExpression;
struct CallExpression;

struct Visitor {
  virtual std::any visit(ASTNode& e) = 0;
  virtual std::any visitProgram(Program& p) = 0;
  virtual std::any visitBlockStatement(BlockStatement& s) = 0;
  virtual std::any visitExpressionStatement(ExpressionStatement& s) = 0;
  virtual std::any visitIfStatement(IfStatement& s) = 0;
  virtual std::any visitPrintStatement(PrintStatement& s) = 0;
  virtual std::any visitReturnStatement(ReturnStatement& s) = 0;
  virtual std::any visitWhileStatement(WhileStatement& s) = 0;
  virtual std::any visitFunctionDeclaration(FunctionDeclaration& s) = 0;
  virtual std::any visitVarDeclaration(VarDeclaration& s) = 0;
  virtual std::any visitConstDeclaration(ConstDeclaration& s) = 0;
  virtual std::any visitLiteralExpression(LiteralExpression& e) = 0;
  virtual std::any visitArrayLiteralExpression(ArrayLiteralExpression& e) = 0;
  virtual std::any visitVarExpression(VarExpression& e) = 0;
  virtual std::any visitArrayAccessExpression(ArrayAccessExpression& e) = 0;
  virtual std::any visitPostfixExpression(PostfixExpression& e) = 0;
  virtual std::any visitUnaryExpression(UnaryExpression& e) = 0;
  virtual std::any visitBinaryExpression(BinaryExpression& e) = 0;
  virtual std::any visitTernaryExpression(TernaryExpression& e) = 0;
  virtual std::any visitAssignmentExpression(AssignmentExpression& e) = 0;
  virtual std::any visitCallExpression(CallExpression& e) = 0;
};