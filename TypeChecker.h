#include <deque>
#include "Visitor.h"
#include "ASTNode.h"
#include "Token.h"
#include "Error.h"
#include "Type.h"

#undef RETURN
#define RETURN(type) do { returnType = type; return; } while (0)

enum class Context {
  FUNCTION,
  LOOP
};

struct : Visitor {
  std::deque<std::unordered_map<std::string, std::pair<Type*, bool>>> stack = {{}};
  std::deque<Context> contexts;
  Type* returnType;

  std::pair<Type*, bool>* getSymbol(Token t) {
    for (auto rit = stack.rbegin(); rit != stack.rend(); ++rit)
      for (auto& pair : *rit)
        if (pair.first == t.lexeme) return &pair.second;

    ReferenceError("'" + t.lexeme + "' has not been declared",
      Source(t.line));
    return nullptr;
  }

  Type* get(ASTNode& e) {
    visit(e);
    return returnType;
  }

  void visit(ASTNode& e) override {
    e.accept(*this);
  }

  void visitProgram(Program& p) override {
    for (Statement* statement : p.statements)
      visit(*statement);
  }

  void visitBlockStatement(BlockStatement& s) override {
    stack.push_back({});

    for (Statement* statement : s.statements)
      visit(*statement);

    stack.pop_back();
  }

  void visitExpressionStatement(ExpressionStatement& s) override {
    visit(s.expr);
  }

  void visitIfStatement(IfStatement& s) override {
    Type* condition = get(s.condition);

    if (condition != ERROR_T && condition != BOOL_T)
      TypeError("'" + condition->toString() + "' cannot be converted to type 'bool'",
        Source(s.condition.start.line));
    
    visit(s.thenBranch);
    if (s.elseBranch) visit(*s.elseBranch);
  }

  void visitPrintStatement(PrintStatement& s) override {
    visit(s.expr);
  }

  void visitReturnStatement(ReturnStatement& s) override {
    if (std::find(contexts.begin(), contexts.end(), Context::FUNCTION) == contexts.end()) {
      SyntaxError("'return' not allowed outside of a function",
        Source(s.token.line));
    }
    if (s.value) visit(*s.value);
  }

  void visitBreakStatement(BreakStatement& s) override {
    if (std::find(contexts.begin(), contexts.end(), Context::LOOP) == contexts.end()) {
      SyntaxError("'break' not allowed outside of a loop",
        Source(s.token.line));
    }
  }

  void visitContinueStatement(ContinueStatement& s) override {
    if (std::find(contexts.begin(), contexts.end(), Context::LOOP) == contexts.end()) {
      SyntaxError("'continue' not allowed outside of a loop",
        Source(s.token.line));
    }
  }

  void visitWhileStatement(WhileStatement& s) override {
    Type* condition = get(s.condition);

    if (condition != ERROR_T && condition != BOOL_T)
      TypeError("'" + condition->toString() + "' cannot be converted to type 'bool'",
        Source(s.condition.start.line));
    
    contexts.push_back(Context::LOOP);
    visit(s.body);
    contexts.pop_back();
  }

  void visitFunctionDeclaration(FunctionDeclaration& s) override {
    std::string id = s.id.lexeme;
    std::vector<Type*> args;
    
    if (stack.back().find(id) != stack.back().end())
      TypeError("'" + id + "' has already been declared", Source(s.id.line));

    for (Argument arg : s.args)
      args.push_back(arg.type);

    stack.back().emplace(
      id,
      std::make_pair(
        std::find(args.begin(), args.end(), ERROR_T) == args.end()?
          new FunctionType(args, s.returnType) : ERROR_T,
        false
      )
    );

    contexts.push_back(Context::FUNCTION);
    stack.push_back({});

    for (Argument arg : s.args) {
      std::string lexeme = arg.id.lexeme;
      if (stack.back().find(lexeme) != stack.back().end())
        TypeError("duplicate paramater '" + lexeme + "' not allowed", Source(arg.id.line));

      stack.back().insert(std::make_pair(lexeme, std::make_pair(arg.type, true)));
    }

    for (Statement* statement : s.body.statements)
      visit(*statement);

    contexts.pop_back();
    stack.pop_back();
  }

  void visitVarDeclaration(VarDeclaration& s) override {
    std::string id = s.id.lexeme;
    Type* valueType = s.initializer? get(*s.initializer) : NULL_T;
    Type* type = valueType == ERROR_T? ERROR_T : s.type? s.type : s.initializer? valueType : ANY_T;

    if (stack.back().find(id) != stack.back().end())
      TypeError("'" + id + "' has already been declared", Source(s.id.line));
    else if (s.type && valueType != ERROR_T && !s.type->superset(valueType)) {
      type = ERROR_T;
      TypeError("cannot assign type '" + valueType->toString() + "' to type '" + s.type->toString() + "'",
        Source(s.id.line));
    }

    stack.back().insert(std::make_pair(id, std::make_pair(type, true)));
  }

  void visitConstDeclaration(ConstDeclaration& s) override {
    std::string id = s.id.lexeme;
    Type* valueType = get(s.initializer);
    Type* type = valueType == ERROR_T? ERROR_T : s.type? s.type : valueType;

    if (stack.back().find(id) != stack.back().end())
      TypeError("'" + id + "' has already been declared", Source(s.id.line));
    else if (s.type && valueType != ERROR_T && !s.type->superset(valueType)) {
      type = ERROR_T;
      TypeError("cannot assign type '" + valueType->toString() + "' to type '" + s.type->toString() + "'",
        Source(s.id.line));
    }

    stack.back().insert(std::make_pair(id, std::make_pair(type, false)));
  }

  void visitLiteralExpression(LiteralExpression& e) override {
    switch (e.value.type) {
      case INT: RETURN(INT_T);
      case FLOAT: RETURN(FLOAT_T);
      case STRING: RETURN(STRING_T);
      case CHAR: RETURN(CHAR_T);
      case TRUE:
      case FALSE: RETURN(BOOL_T);
      default: RETURN(NULL_T);
    }
  }

  void visitArrayLiteralExpression(ArrayLiteralExpression& e) override {
    if (e.elements.empty()) RETURN(new ArrayType(NULL_T));

    UnionType* elementsType = new UnionType({});

    for (Expression* element : e.elements) {
      Type* type = get(*element);

      if (type == ERROR_T) RETURN(ERROR_T);

      elementsType->add(type);
    }

    if (elementsType->types.size() == 1) RETURN(new ArrayType(elementsType->types[0]));
    RETURN(new ArrayType(elementsType));
  }

  void visitVarExpression(VarExpression& e) override {
    auto symbol = getSymbol(e.id);
    RETURN(symbol? symbol->first : ERROR_T);
  }

  void visitCallExpression(CallExpression& e) override {
    Type* callee = get(e.callee);

    if (callee == ERROR_T) RETURN(ERROR_T);

    std::vector<Type*> args;

    for (Expression* item : e.args) {
      Type* arg = get(*item);

      if (arg == ERROR_T) RETURN(ERROR_T);

      args.push_back(arg);
    }

    FunctionType* function = dynamic_cast<FunctionType*>(callee);

    if (!function) {
      TypeError("cannot call type '" + callee->toString() + "'",
        Source(e.callee.start.line));
      RETURN(ERROR_T);
    }
    else if (!function->equals(new FunctionType(args, function->returnType))) {
      TypeError("argument mismatch",
        Source(e.callee.start.line));
      RETURN(ERROR_T);
    }

    RETURN(function->returnType);
  }

  void visitArrayAccessExpression(ArrayAccessExpression& e) override {
    Type* type = get(e.array);
    Type* indexType = get(e.index);

    if (type == ERROR_T || indexType == ERROR_T) RETURN(ERROR_T);

    ArrayType* arrayType = dynamic_cast<ArrayType*>(type);

    if (!arrayType) {
      TypeError("cannot perform an array access on type '" + type->toString() + "'",
        Source(e.array.start.line));
      RETURN(ERROR_T);
    }
    else if (indexType != INT_T) {
      TypeError("array index must be of type 'int'", Source(e.index.start.line));
      RETURN(ERROR_T);
    }

    RETURN(arrayType->elementsType);
  }

  void visitCastExpression(CastExpression& e) override {
    Type* type = get(e.expr);

    if (type == ERROR_T) RETURN(ERROR_T);
    else if (!type->superset(e.type)) {
      TypeError("cannot cast type '" + type->toString() + "' to type '" + e.type->toString() + "'",
        Source(e.expr.start.line));
      RETURN(ERROR_T);
    }

    RETURN(e.type);
  }

  void visitPostfixExpression(PostfixExpression& e) override {
    Type* type = get(e.expr);

    if (type == ERROR_T) RETURN(ERROR_T);

    if (VarExpression* expr = dynamic_cast<VarExpression*>(&e.expr)) {
      if (!getSymbol(expr->id)->second) {
        TypeError("cannot reassign '" + expr->id.lexeme + "'",
          Source(e.expr.start.line));
        RETURN(ERROR_T);
      }
    }
    else if (!dynamic_cast<ArrayAccessExpression*>(&e.expr)) {
      TypeError("cannot perform operation '" + e.op.lexeme + "' on an r-value",
        Source(e.expr.start.line));
      RETURN(ERROR_T);
    }

    if (type == INT_T) RETURN(INT_T);
    else if (type == FLOAT_T) RETURN(FLOAT_T);

    TypeError("cannot perform operation '" + e.op.lexeme + "' on type '" + type->toString() + "'",
      Source(e.expr.start.line));
    RETURN(ERROR_T);
  }

  void visitUnaryExpression(UnaryExpression& e) override {
    Type* type = get(e.expr);

    if (type == ERROR_T) RETURN(ERROR_T);

    switch (e.op.type) {
      case PLUS:
      case MINUS:
        if (type == INT_T) RETURN(INT_T);
        else if (type == FLOAT_T) RETURN(FLOAT_T);
        
        TypeError("cannot perform operation '" + e.op.lexeme + "' on type '" + type->toString() + "'",
          Source(e.expr.start.line));
        RETURN(ERROR_T);

      case INCREMENT:
      case DECREMENT:
        if (VarExpression* expr = dynamic_cast<VarExpression*>(&e.expr)) {
          if (!getSymbol(expr->id)->second) {
            TypeError("cannot reassign '" + expr->id.lexeme + "'",
              Source(e.expr.start.line));
            RETURN(ERROR_T);
          }
        }
        else if (!dynamic_cast<ArrayAccessExpression*>(&e.expr)) {
          TypeError("cannot perform operation '" + e.op.lexeme + "' on an r-value",
            Source(e.expr.start.line));
          RETURN(ERROR_T);
        }

        if (type == INT_T) RETURN(INT_T);
        else if (type == FLOAT_T) RETURN(FLOAT_T);

        TypeError("cannot perform operation '" + e.op.lexeme + "' on type '" + type->toString() + "'",
          Source(e.expr.start.line));
        RETURN(ERROR_T);

      default:
        if (type == BOOL_T) RETURN(BOOL_T);
        
        TypeError("cannot perform operation '!' on type '" + type->toString() + "'",
          Source(e.expr.start.line));
        RETURN(ERROR_T);
    }
  }

  void visitBinaryExpression(BinaryExpression& e) override {
    Type* left = get(e.left);
    Type* right = get(e.right);

    if (left == ERROR_T || right == ERROR_T) RETURN(ERROR_T);

    switch (e.op.type) {
      // TODO: make `new UnionType({ left, right })` exclude NULL_T
      case COALESCENCE: RETURN(!left->superset(NULL_T)? left :
        left == NULL_T? right : new UnionType({ left, right }));
      case AND:
      case OR:
        if (left == BOOL_T && right == BOOL_T) RETURN(BOOL_T);

        TypeError("cannot perform operation '" + e.op.lexeme + "' on types '" + left->toString() + "' and '" + right->toString() + "'",
          Source(e.op.line));
        RETURN(ERROR_T);

      case EQUALS:
      case NOT_EQUALS: RETURN(BOOL_T);
      case GREATER:
      case LESS:
      case GREATER_EQUALS:
      case LESS_EQUALS:
        if (left == INT_T && right == INT_T || left == FLOAT_T && right == FLOAT_T) RETURN(BOOL_T);
          
        TypeError("cannot perform operation '" + e.op.lexeme + "' on types '" + left->toString() + "' and '" + right->toString() + "'",
          Source(e.op.line));
        RETURN(ERROR_T);

      case PLUS:
        if (left == INT_T && right == INT_T) RETURN(INT_T);
        else if (left == FLOAT_T && right == FLOAT_T) RETURN(FLOAT_T);
        else if (left == STRING_T && right == STRING_T) RETURN(STRING_T);

        TypeError("cannot perform operation '+' on types '" + left->toString() + "' and '" + right->toString() + "'",
          Source(e.op.line));
        RETURN(ERROR_T);

      default:
        if (left == INT_T && right == INT_T) RETURN(INT_T);
        else if (left == FLOAT_T && right == FLOAT_T) RETURN(FLOAT_T);

        TypeError("cannot perform operation '" + e.op.lexeme + "' on types '" + left->toString() + "' and '" + right->toString() + "'",
          Source(e.op.line));
        RETURN(ERROR_T);
    }
  }

  void visitTernaryExpression(TernaryExpression& e) override {
    Type* conditionType = get(e.condition);
    Type* valueType = get(e.value);
    Type* defaultType = get(e._default);

    if (conditionType == ERROR_T || valueType == ERROR_T || defaultType == ERROR_T) RETURN(ERROR_T);

    if (conditionType != BOOL_T) {
      TypeError("condition expression must be of type 'bool'",
        Source(e.condition.start.line));
      RETURN(ERROR_T);
    }

    RETURN(valueType == defaultType?
      valueType : new UnionType({ valueType, defaultType }));
  }

  void visitAssignmentExpression(AssignmentExpression& e) override {
    Type* left;

    if (VarExpression* expr = dynamic_cast<VarExpression*>(&e.l_value)) {
      auto symbol = getSymbol(expr->id);

      if (!symbol) RETURN(ERROR_T);
      else if (!symbol->second) {
        TypeError("cannot reassign '" + expr->id.lexeme + "'",
          Source(e.l_value.start.line));
        RETURN(ERROR_T);
      }

      left = symbol->first;
    }
    else if (!dynamic_cast<ArrayAccessExpression*>(&e.l_value)) {
      TypeError("cannot perform operation '" + e.op.lexeme + "' on an r-value",
        Source(e.l_value.start.line));
      RETURN(ERROR_T);
    }
    else left = get(e.l_value);

    Type* right = get(e.value);

    if (left == ERROR_T || right == ERROR_T) RETURN(ERROR_T);

    switch (e.op.type) {
      case ASSIGN:
        if (left->superset(right)) RETURN(right);

        TypeError("cannot perform operation '" + e.op.lexeme + "' on types '" + left->toString() + "' and '" + right->toString() + "'",
          Source(e.value.start.line));
        RETURN(ERROR_T);

      case PLUS_EQUALS:
        if (left == INT_T && right == INT_T) RETURN(INT_T);
        else if (left == FLOAT_T && right == FLOAT_T) RETURN(FLOAT_T);
        else if (left == STRING_T && right == STRING_T) RETURN(STRING_T);
        
        TypeError("cannot perform operation '+=' on types '" + left->toString() + "' and '" + right->toString() + "'",
          Source(e.value.start.line));
        RETURN(ERROR_T);

      default:
        if (left == INT_T && right == INT_T) RETURN(INT_T);
        else if (left == FLOAT_T && right == FLOAT_T) RETURN(FLOAT_T);

        TypeError("cannot perform operation '" + e.op.lexeme + "' on types '" + left->toString() + "' and '" + right->toString() + "'",
          Source(e.value.start.line));
        RETURN(ERROR_T);
    }
  }
} TypeChecker;