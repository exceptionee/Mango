#include <deque>
#include <any>
#include "Visitor.h"
#include "ASTNode.h"
#include "Token.h"
#include "Error.h"
#include "Type.h"

struct Symbol {
  enum Mutability { VAR, CONST };

  Mutability mutability;
  Type* type;

  Symbol(Mutability mutability, Type* type) : mutability(mutability), type(type) {}
};

struct : Visitor {
  std::deque<std::unordered_map<std::string, Symbol>> stack = {{}};

  Symbol* getSymbol(Token t) {
    for (auto rit = stack.rbegin(); rit != stack.rend(); ++rit)
      for (auto& pair : *rit)
        if (pair.first == t.lexeme) return &pair.second;

    ReferenceError("'" + std::string(t.lexeme) + "' has not been declared",
      Source(t.line));
    return nullptr;
  }

  std::any visit(ASTNode& e) override {
    return e.accept(this);
  }

  std::any visitProgram(Program& p) override {
    for (Statement* statement : p.statements)
      visit(*statement);
      
    return std::any{};
  }

  std::any visitBlockStatement(BlockStatement& s) override {
    stack.push_back({});

    for (Statement* statement : s.statements)
      visit(*statement);

    stack.pop_back();
    return std::any{};
  }

  std::any visitExpressionStatement(ExpressionStatement& s) override {
    visit(s.expr);
    return std::any{};
  }

  std::any visitPrintStatement(PrintStatement& s) override {
    visit(s.expr);
    return std::any{};
  }

  std::any visitVarDeclarationStatement(VarDeclarationStatement& s) override {
    std::string id = std::string(s.id.lexeme);
    Type* valueType = s.initializer != nullptr?
      std::any_cast<Type*>(visit(*s.initializer)) : NULL_T;
    Type* type = valueType == ERROR_T? ERROR_T :
      s.type != nullptr? s.type : s.initializer != nullptr? valueType : ANY_T;

    if (stack.back().find(id) != stack.back().end())
      TypeError("'" + id + "' has already been declared", Source(s.id.line));
    else if (s.type != nullptr && valueType != ERROR_T && !s.type->superset(valueType)) {
      type = ERROR_T;
      TypeError("cannot assign type '" + valueType->toString() + "' to type '" + s.type->toString() + "'",
        Source(s.id.line));
    }

    stack.back().insert(std::make_pair(id, Symbol(Symbol::VAR, type)));
    return std::any{};
  }

  std::any visitConstDeclarationStatement(ConstDeclarationStatement& s) override {
    std::string id = std::string(s.id.lexeme);
    Type* valueType = std::any_cast<Type*>(visit(s.initializer));
    Type* type = valueType == ERROR_T? ERROR_T :
      s.type != nullptr? s.type : valueType;

    if (stack.back().find(id) != stack.back().end())
      TypeError("'" + id + "' has already been declared", Source(s.id.line));
    else if (s.type != nullptr && valueType != ERROR_T && !s.type->superset(valueType)) {
      type = ERROR_T;
      TypeError("cannot assign type '" + valueType->toString() + "' to type '" + s.type->toString() + "'",
        Source(s.id.line));
    }

    stack.back().insert(std::make_pair(id, Symbol(Symbol::CONST, type)));
    return std::any{};
  }

  std::any visitLiteralExpression(LiteralExpression& e) override {
    switch (e.value.type) {
      case INT: return INT_T;
      case FLOAT: return FLOAT_T;
      case STRING: return STRING_T;
      case TRUE:
      case FALSE: return BOOL_T;
      default: return NULL_T;
    }
  }

  std::any visitArrayLiteralExpression(ArrayLiteralExpression& e) override {}

  std::any visitVarExpression(VarExpression& e) override {
    Symbol* symbol = getSymbol(e.id);
    return symbol != nullptr? symbol->type : ERROR_T;
  }

  std::any visitArrayAccessExpression(ArrayAccessExpression& e) override {}

  std::any visitPostfixExpression(PostfixExpression& e) override {}

  std::any visitUnaryExpression(UnaryExpression& e) override {
    Type* type = std::any_cast<Type*>(visit(e.expr));

    if (type == ERROR_T) return ERROR_T;

    switch (e.op.type) {
      case PLUS:
      case MINUS:
        if (type == INT_T) return INT_T;
        else if (type == FLOAT_T) return FLOAT_T;
        
        TypeError("cannot perform unary operation '" + std::string(e.op.lexeme) + "' on type '" + type->toString() + "'",
          Source(e.op.line));
        return ERROR_T;

      case INCREMENT:
      case DECREMENT:
        if (VarExpression* expr = dynamic_cast<VarExpression*>(&e.expr)) {
          if (getSymbol(expr->id)->mutability == Symbol::CONST) {
            TypeError("cannot reassign constant '" + std::string(expr->id.lexeme) + "'",
              Source(e.op.line));
            return ERROR_T;
          }
        }
        else if (dynamic_cast<ArrayAccessExpression*>(&e.expr) == nullptr) {
          TypeError("invalid right side for unary operator '" + std::string(e.op.lexeme) + "'",
            Source(e.op.line));
          return ERROR_T;
        }

        if (type == INT_T) return INT_T;
        else if (type == FLOAT_T) return FLOAT_T;

        TypeError("cannot perform unary operation '" + std::string(e.op.lexeme) + "' on type '" + type->toString() + "'",
          Source(e.op.line));
        return ERROR_T;

      default:
        if (type == BOOL_T) return BOOL_T;
        
        TypeError("cannot perform unary operation '!' on type '" + type->toString() + "'",
          Source(e.op.line));
        return ERROR_T;
    }
  }

  std::any visitBinaryExpression(BinaryExpression& e) override {
    Type* left = std::any_cast<Type*>(visit(e.left));
    Type* right = std::any_cast<Type*>(visit(e.right));

    if (left == ERROR_T || right == ERROR_T) return ERROR_T;

    switch (e.op.type) {
      case COALESCENCE: return NULL_T;
      case AND:
      case OR:
        if (left != BOOL_T || right != BOOL_T) {
          TypeError("cannot perform operation '" + std::string(e.op.lexeme) + "' on types '" + left->toString() + "' and '" + right->toString() + "'",
            Source(e.op.line));
          return ERROR_T;
        }

        return BOOL_T;
      case EQUALS:
      case NOT_EQUALS: return BOOL_T;
      case PLUS:
        if (left == INT_T && right == INT_T) return INT_T;
        else if (left == FLOAT_T && right == FLOAT_T) return FLOAT_T;
        else if (left == STRING_T && right == STRING_T) return STRING_T;

        TypeError("cannot perform operation '+' on types '" + left->toString() + "' and '" + right->toString() + "'",
          Source(e.op.line));
        return ERROR_T;
      default:
        if (left == INT_T && right == INT_T) return INT_T;
        else if (left == FLOAT_T && right == FLOAT_T) return FLOAT_T;

        TypeError("cannot perform operation '" + std::string(e.op.lexeme) + "' on types '" + left->toString() + "' and '" + right->toString() + "'",
          Source(e.op.line));
        return ERROR_T;
    }
  }

  std::any visitTernaryExpression(TernaryExpression& e) override {}

  std::any visitAssignmentExpression(AssignmentExpression& e) override {
    Type* left;

    if (VarExpression* expr = dynamic_cast<VarExpression*>(&e.l_value)) {
      Symbol* symbol = getSymbol(expr->id);

      if (symbol == nullptr) return ERROR_T;
      else if (symbol->mutability == Symbol::CONST) {
        TypeError("cannot reassign constant '" + std::string(expr->id.lexeme) + "'",
          Source(e.op.line));
        return ERROR_T;
      }

      left = symbol->type;
    }
    else if (dynamic_cast<ArrayAccessExpression*>(&e.l_value) == nullptr) {
      TypeError("invalid left side for operator '" + std::string(e.op.lexeme) + "'",
        Source(e.op.line));
      return ERROR_T;
    }
    else left = std::any_cast<Type*>(visit(e.l_value));

    Type* right = std::any_cast<Type*>(visit(e.value));

    if (left == ERROR_T || right == ERROR_T) return ERROR_T;

    switch (e.op.type) {
      case ASSIGN:
        if (left->superset(right)) return right;

        TypeError("cannot assign type '" + right->toString() + "' to type '" + left->toString() + "'",
          Source(e.op.line));
        return ERROR_T;

      case PLUS_EQUALS:
        if (left == INT_T && right == INT_T) return INT_T;
        else if (left == FLOAT_T && right == FLOAT_T) return FLOAT_T;
        else if (left == STRING_T && right == STRING_T) return STRING_T;
        
        TypeError("cannot perform operation '+=' on types '" + left->toString() + "' and '" + right->toString() + "'",
          Source(e.op.line));
        return ERROR_T;

      default:
        if (left == INT_T && right == INT_T) return INT_T;
        else if (left == FLOAT_T && right == FLOAT_T) return FLOAT_T;

        TypeError("cannot perform operation '" + std::string(e.op.lexeme) + "' on types '" + left->toString() + "' and '" + right->toString() + "'",
          Source(e.op.line));
        return ERROR_T;
    }
  }
} TypeChecker;