#include <deque>
#include <any>
#include "Visitor.h"
#include "ASTNode.h"
#include "Token.h"
#include "Error.h"
#include "Type.h"

struct Symbol {
  enum Declaration { FUNCTION, VAR, CONST };

  Declaration declaration;
  Type* type;

  Symbol(Declaration declaration, Type* type) : declaration(declaration), type(type) {}
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
    return e.accept(*this);
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

  std::any visitIfStatement(IfStatement& s) override {
    Type* condition = std::any_cast<Type*>(visit(s.condition));

    if (condition != ERROR_T && condition != BOOL_T)
      TypeError("'" + condition->toString() + "' cannot be converted to type 'bool'",
        Source(s.condition.start.line));
    
    visit(s.thenBranch);
    if (s.elseBranch) visit(*s.elseBranch);
    return std::any{};
  }

  std::any visitPrintStatement(PrintStatement& s) override {
    visit(s.expr);
    return std::any{};
  }

  std::any visitReturnStatement(ReturnStatement& s) override {
    if (s.value) visit(*s.value);
    return std::any{};
  }

  std::any visitWhileStatement(WhileStatement& s) override {
    Type* condition = std::any_cast<Type*>(visit(s.condition));

    if (condition != ERROR_T && condition != BOOL_T)
      TypeError("'" + condition->toString() + "' cannot be converted to type 'bool'",
        Source(s.condition.start.line));
    
    visit(s.body);
    return std::any{};
  }

  std::any visitFunctionDeclaration(FunctionDeclaration& s) override {
    std::string id = std::string(s.id.lexeme);
    std::vector<Type*> args;
    
    if (stack.back().find(id) != stack.back().end())
      TypeError("'" + id + "' has already been declared", Source(s.id.line));

    for (Argument arg : s.args)
      args.push_back(arg.type);

    stack.back().insert(std::make_pair(id, Symbol(Symbol::FUNCTION,
      std::find(args.begin(), args.end(), ERROR_T) == args.end()?
        new FunctionType(args, s.returnType) : ERROR_T
    )));

    stack.push_back({});

    for (Argument arg : s.args) {
      std::string lexeme = std::string(arg.id.lexeme);
      if (stack.back().find(lexeme) != stack.back().end())
        TypeError("duplicate paramater '" + lexeme + "' not allowed", Source(arg.id.line));

      stack.back().insert(std::make_pair(lexeme, Symbol(Symbol::VAR, arg.type)));
    }

    for (Statement* statement : s.body.statements)
      visit(*statement);

    stack.pop_back();
    return std::any{};
  }

  std::any visitVarDeclaration(VarDeclaration& s) override {
    std::string id = std::string(s.id.lexeme);
    Type* valueType = s.initializer? std::any_cast<Type*>(visit(*s.initializer)) : NULL_T;
    Type* type = valueType == ERROR_T? ERROR_T : s.type? s.type : s.initializer? valueType : ANY_T;

    if (stack.back().find(id) != stack.back().end())
      TypeError("'" + id + "' has already been declared", Source(s.id.line));
    else if (s.type && valueType != ERROR_T && !s.type->superset(valueType)) {
      type = ERROR_T;
      TypeError("cannot assign type '" + valueType->toString() + "' to type '" + s.type->toString() + "'",
        Source(s.id.line));
    }

    stack.back().insert(std::make_pair(id, Symbol(Symbol::VAR, type)));
    return std::any{};
  }

  std::any visitConstDeclaration(ConstDeclaration& s) override {
    std::string id = std::string(s.id.lexeme);
    Type* valueType = std::any_cast<Type*>(visit(s.initializer));
    Type* type = valueType == ERROR_T? ERROR_T : s.type? s.type : valueType;

    if (stack.back().find(id) != stack.back().end())
      TypeError("'" + id + "' has already been declared", Source(s.id.line));
    else if (s.type && valueType != ERROR_T && !s.type->superset(valueType)) {
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
      case CHAR: return CHAR_T;
      case TRUE:
      case FALSE: return BOOL_T;
      default: return NULL_T;
    }
  }

  std::any visitArrayLiteralExpression(ArrayLiteralExpression& e) override {
    if (e.elements.empty()) return static_cast<Type*>(new ArrayType(NULL_T));

    UnionType* elementsType = new UnionType({});

    for (Expression* element : e.elements) {
      Type* type = std::any_cast<Type*>(visit(*element));

      if (type == ERROR_T) return ERROR_T;

      elementsType->add(type);
    }

    if (elementsType->types.size() == 1)
      return static_cast<Type*>(new ArrayType(elementsType->types[0]));
    return static_cast<Type*>(new ArrayType(elementsType));
  }

  std::any visitVarExpression(VarExpression& e) override {
    Symbol* symbol = getSymbol(e.id);
    return symbol? symbol->type : ERROR_T;
  }

  std::any visitCallExpression(CallExpression& e) override {
    Type* callee = std::any_cast<Type*>(visit(e.callee));

    if (callee == ERROR_T) return ERROR_T;

    std::vector<Type*> args;

    for (Expression* item : e.args) {
      Type* arg = std::any_cast<Type*>(visit(*item));

      if (arg == ERROR_T) return ERROR_T;
      args.push_back(arg);
    }

    FunctionType* function = dynamic_cast<FunctionType*>(callee);

    if (!function) {
      TypeError("cannot call type '" + callee->toString() + "'",
        Source(e.closeParen.line));
      return ERROR_T;
    }
    else if (!function->equals(new FunctionType(args, function->returnType))) {
      TypeError("argument mismatch",
        Source(e.closeParen.line));
      return ERROR_T;
    }

    return function->returnType;
  }

  std::any visitArrayAccessExpression(ArrayAccessExpression& e) override {
    Type* type = std::any_cast<Type*>(visit(e.array));
    Type* indexType = std::any_cast<Type*>(visit(e.index));

    if (type == ERROR_T || indexType == ERROR_T) return ERROR_T;

    ArrayType* arrayType = dynamic_cast<ArrayType*>(type);

    if (!arrayType) {
      TypeError("cannot perform an array access on type '" + type->toString() + "'",
        Source(e.array.start.line));
      return ERROR_T;
    }
    else if (indexType != INT_T) {
      TypeError("array index must be of type 'int'", Source(e.index.start.line));
      return ERROR_T;
    }

    return arrayType->elementsType;
  }

  std::any visitCastExpression(CastExpression& e) override {
    Type* type = std::any_cast<Type*>(visit(e.expr));

    if (type == ERROR_T) return ERROR_T;
    else if (!type->superset(e.type)) {
      TypeError("cannot cast type '" + type->toString() + "' to type '" + e.type->toString() + "'",
        Source(e.expr.start.line));
      return ERROR_T;
    }

    return e.type;
  }

  std::any visitPostfixExpression(PostfixExpression& e) override {
    Type* type = std::any_cast<Type*>(visit(e.expr));

    if (type == ERROR_T) return ERROR_T;

    if (VarExpression* expr = dynamic_cast<VarExpression*>(&e.expr)) {
      if (getSymbol(expr->id)->declaration != Symbol::VAR) {
        TypeError("cannot reassign '" + std::string(expr->id.lexeme) + "'",
          Source(e.expr.start.line));
        return ERROR_T;
      }
    }
    else if (!dynamic_cast<ArrayAccessExpression*>(&e.expr)) {
      TypeError("cannot perform operation '" + std::string(e.op.lexeme) + "' on an r-value",
        Source(e.expr.start.line));
      return ERROR_T;
    }

    if (type == INT_T) return INT_T;
    else if (type == FLOAT_T) return FLOAT_T;

    TypeError("cannot perform operation '" + std::string(e.op.lexeme) + "' on type '" + type->toString() + "'",
      Source(e.expr.start.line));
    return ERROR_T;
  }

  std::any visitUnaryExpression(UnaryExpression& e) override {
    Type* type = std::any_cast<Type*>(visit(e.expr));

    if (type == ERROR_T) return ERROR_T;

    switch (e.op.type) {
      case PLUS:
      case MINUS:
        if (type == INT_T) return INT_T;
        else if (type == FLOAT_T) return FLOAT_T;
        
        TypeError("cannot perform operation '" + std::string(e.op.lexeme) + "' on type '" + type->toString() + "'",
          Source(e.expr.start.line));
        return ERROR_T;

      case INCREMENT:
      case DECREMENT:
        if (VarExpression* expr = dynamic_cast<VarExpression*>(&e.expr)) {
          if (getSymbol(expr->id)->declaration != Symbol::VAR) {
            TypeError("cannot reassign '" + std::string(expr->id.lexeme) + "'",
              Source(e.expr.start.line));
            return ERROR_T;
          }
        }
        else if (!dynamic_cast<ArrayAccessExpression*>(&e.expr)) {
          TypeError("cannot perform operation '" + std::string(e.op.lexeme) + "' on an r-value",
            Source(e.expr.start.line));
          return ERROR_T;
        }

        if (type == INT_T) return INT_T;
        else if (type == FLOAT_T) return FLOAT_T;

        TypeError("cannot perform operation '" + std::string(e.op.lexeme) + "' on type '" + type->toString() + "'",
          Source(e.expr.start.line));
        return ERROR_T;

      default:
        if (type == BOOL_T) return BOOL_T;
        
        TypeError("cannot perform operation '!' on type '" + type->toString() + "'",
          Source(e.expr.start.line));
        return ERROR_T;
    }
  }

  std::any visitBinaryExpression(BinaryExpression& e) override {
    Type* left = std::any_cast<Type*>(visit(e.left));
    Type* right = std::any_cast<Type*>(visit(e.right));

    if (left == ERROR_T || right == ERROR_T) return ERROR_T;

    switch (e.op.type) {
      // TODO: make `new UnionType({ left, right })` exclude NULL_T
      case COALESCENCE: return !left->superset(NULL_T)? left :
        left == NULL_T? right : new UnionType({ left, right });
      case AND:
      case OR:
        if (left == BOOL_T && right == BOOL_T) return BOOL_T;

        TypeError("cannot perform operation '" + std::string(e.op.lexeme) + "' on types '" + left->toString() + "' and '" + right->toString() + "'",
          Source(e.op.line));
        return ERROR_T;

      case EQUALS:
      case NOT_EQUALS: return BOOL_T;
      case GREATER:
      case LESS:
      case GREATER_EQUALS:
      case LESS_EQUALS:
        if (left == INT_T && right == INT_T || left == FLOAT_T && right == FLOAT_T)
          return BOOL_T;
          
        TypeError("cannot perform operation '" + std::string(e.op.lexeme) + "' on types '" + left->toString() + "' and '" + right->toString() + "'",
          Source(e.op.line));
        return ERROR_T;

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

  std::any visitTernaryExpression(TernaryExpression& e) override {
    Type* conditionType = std::any_cast<Type*>(visit(e.condition));
    Type* valueType = std::any_cast<Type*>(visit(e.value));
    Type* defaultType = std::any_cast<Type*>(visit(e._default));

    if (conditionType == ERROR_T || valueType == ERROR_T || defaultType == ERROR_T)
      return ERROR_T;

    if (conditionType != BOOL_T) {
      TypeError("condition expression must be of type 'bool'",
        Source(e.condition.start.line));
      return ERROR_T;
    }

    return valueType == defaultType?
      valueType : new UnionType({ valueType, defaultType });
  }

  std::any visitAssignmentExpression(AssignmentExpression& e) override {
    Type* left;

    if (VarExpression* expr = dynamic_cast<VarExpression*>(&e.l_value)) {
      Symbol* symbol = getSymbol(expr->id);

      if (!symbol) return ERROR_T;
      else if (symbol->declaration != Symbol::VAR) {
        TypeError("cannot reassign '" + std::string(expr->id.lexeme) + "'",
          Source(e.l_value.start.line));
        return ERROR_T;
      }

      left = symbol->type;
    }
    else if (!dynamic_cast<ArrayAccessExpression*>(&e.l_value)) {
      TypeError("cannot perform operation '" + std::string(e.op.lexeme) + "' on an r-value",
        Source(e.l_value.start.line));
      return ERROR_T;
    }
    else left = std::any_cast<Type*>(visit(e.l_value));

    Type* right = std::any_cast<Type*>(visit(e.value));

    if (left == ERROR_T || right == ERROR_T) return ERROR_T;

    switch (e.op.type) {
      case ASSIGN:
        if (left->superset(right)) return right;

        TypeError("cannot perform operation '" + std::string(e.op.lexeme) + "' on types '" + left->toString() + "' and '" + right->toString() + "'",
          Source(e.value.start.line));
        return ERROR_T;

      case PLUS_EQUALS:
        if (left == INT_T && right == INT_T) return INT_T;
        else if (left == FLOAT_T && right == FLOAT_T) return FLOAT_T;
        else if (left == STRING_T && right == STRING_T) return STRING_T;
        
        TypeError("cannot perform operation '+=' on types '" + left->toString() + "' and '" + right->toString() + "'",
          Source(e.value.start.line));
        return ERROR_T;

      default:
        if (left == INT_T && right == INT_T) return INT_T;
        else if (left == FLOAT_T && right == FLOAT_T) return FLOAT_T;

        TypeError("cannot perform operation '" + std::string(e.op.lexeme) + "' on types '" + left->toString() + "' and '" + right->toString() + "'",
          Source(e.value.start.line));
        return ERROR_T;
    }
  }
} TypeChecker;