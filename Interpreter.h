#pragma once

#include <charconv>
#include <iostream>
#include <math.h>
#include <system_error>
#include "ASTNode.h"
#include "Error.h"
#include "Value.h"
#include "Environment.h"

#undef RETURN
#define RETURN(value) do { returnValue = value; return; } while (0)

#define MATH_OP(left, op, right) std::holds_alternative<long long>(left.data)? \
  Value{std::get<long long>(left.data) op std::get<long long>(right.data)} : \
  Value{std::get<double>(left.data) op std::get<double>(right.data)}

struct Return {
  Value value;
  
  Return(Value value) : value(value) {}
};
struct Break {};
struct Continue {};

struct : Visitor {
  std::shared_ptr<Environment> globals = std::make_shared<Environment>(nullptr);
  std::shared_ptr<Environment> current = globals;
  Value returnValue;

  Value& getLValue(Expression& e) {
    if (VarExpression* varExpr = dynamic_cast<VarExpression*>(&e))
      return current->get(varExpr->id.lexeme);

    ArrayAccessExpression* arrayAccess = dynamic_cast<ArrayAccessExpression*>(&e);
    std::vector<Value>* elements = &std::static_pointer_cast<Array>
      (std::get<std::shared_ptr<Object>>(eval(arrayAccess->array).data))->elements;
    const long long index = std::get<long long>(eval(arrayAccess->index).data);

    if (index < 0 || index >= elements->size()) {
      throw RuntimeError(
        "index " + std::to_string(index) + " out of bounds for length " + std::to_string(elements->size()),
        Source(arrayAccess->index.start.line)
      );
    }

    return (*elements)[index];
  }

  Value eval(ASTNode& e) {
    visit(e);
    return returnValue;
  }

  void visit(ASTNode& e) override {
    e.accept(*this);
  }

  void visitProgram(Program& p) override {
    if (p.statements.size() > 0) {
      try {
        for (int i = 0; i < p.statements.size() - 1; ++i)
          visit(*p.statements[i]);
  
        const auto result = eval(*p.statements[p.statements.size() - 1]);
  
        if (dynamic_cast<ExpressionStatement*>(p.statements[p.statements.size() - 1]))
          RETURN(result);
      }
      catch (RuntimeError e) {}
    }

    RETURN(Value{std::monostate{}});
  }

  void visitBlockStatement(BlockStatement& s) override {
    std::shared_ptr<Environment> previous = current;
    current = std::make_shared<Environment>(current);

    for (Statement* statement : s.statements)
      visit(*statement);

    current = previous;
  }

  void visitExpressionStatement(ExpressionStatement& s) override {
    RETURN(eval(s.expr));
  }

  void visitIfStatement(IfStatement& s) override  {
    if (std::get<bool>(eval(s.condition).data))
      visit(s.thenBranch);
    else if (s.elseBranch)
      visit(*s.elseBranch);
  }

  void visitPrintStatement(PrintStatement& s) override {
    std::cout << eval(s.expr).toString() << std::endl;
  }

  void visitReturnStatement(ReturnStatement& s) override {
    throw Return(s.value? eval(*s.value) : Value{std::monostate{}});
  }

  void visitBreakStatement(BreakStatement& s) override {
    throw Break();
  }

  void visitContinueStatement(ContinueStatement& s) override {
    throw Continue();
  }

  void visitWhileStatement(WhileStatement& s) override {
    while (std::get<bool>(eval(s.condition).data)) {
      try {
        visit(s.body);
      }
      catch (Continue) {}
      catch (Break) { break; }
    }
  }

  void visitFunctionDeclaration(FunctionDeclaration& s) override {
    current = std::make_shared<Environment>(current);
    current->set(s.id.lexeme, Value{std::make_shared<Function>(s, current)});
  }

  void visitVarDeclaration(VarDeclaration& s) override {
    current = std::make_shared<Environment>(current);
    current->set(
      s.id.lexeme,
      s.initializer? eval(*s.initializer) : Value{std::monostate{}}
    );
  }

  void visitConstDeclaration(ConstDeclaration& s) override {
    current = std::make_shared<Environment>(current);
    current->set(s.id.lexeme, eval(s.initializer));
  }

  void visitLiteralExpression(LiteralExpression& e) override {
    switch (e.value.type) {
      case INT: {
        long long result;
        auto [_, ec] = std::from_chars(e.value.lexeme.data(), e.value.lexeme.data() + e.value.lexeme.size(), result);
        
        if (ec == std::errc::result_out_of_range)
          throw RuntimeError("number " + e.value.lexeme + " out of range for type 'int'", Source(e.value.line));

        RETURN(Value{result});
      }
      case FLOAT: {
        double result;
        auto [_, ec] = std::from_chars(e.value.lexeme.data(), e.value.lexeme.data() + e.value.lexeme.size(), result);
        
        if (ec == std::errc::result_out_of_range)
          throw RuntimeError("number " + e.value.lexeme + " out of range for type 'float'", Source(e.value.line));

        RETURN(Value{result});
      }
      case STRING: RETURN(Value{std::make_shared<String>(e.value.lexeme)});
      case CHAR: RETURN(Value{e.value.lexeme[0]});
      case TRUE: RETURN(Value{true});
      case FALSE: RETURN(Value{false});
      default: RETURN(Value{std::monostate{}});
    }
  }

  void visitArrayLiteralExpression(ArrayLiteralExpression& e) override {
    std::vector<Value> elements;

    for (Expression* element : e.elements)
      elements.push_back(eval(*element));

    RETURN(Value{std::make_shared<Array>(elements)});
  }

  void visitVarExpression(VarExpression& e) override {
    RETURN(current->get(e.id.lexeme));
  }

  void visitCallExpression(CallExpression& e) override {
    Value callee = eval(e.callee);
    std::shared_ptr<Function> function = std::static_pointer_cast<Function>(std::get<std::shared_ptr<Object>>(callee.data));

    std::shared_ptr<Environment> env = std::make_shared<Environment>(function->closure);

    for (int i = 0; i < function->declaration.args.size(); ++i)
      env->set(function->declaration.args[i].id.lexeme, eval(*e.args[i]));

    std::shared_ptr<Environment> previous = current;
    current = env;

    try {
      for (Statement* statement : function->declaration.body.statements)
        visit(*statement);
    }
    catch(Return r) {
      current = previous;
      RETURN(r.value);
    }

    current = previous;
    RETURN(Value{std::monostate{}});
  }

  void visitArrayAccessExpression(ArrayAccessExpression& e) override {
    const std::vector<Value> elements = std::static_pointer_cast<Array>
      (std::get<std::shared_ptr<Object>>(eval(e.array).data))->elements;
    const long long index = std::get<long long>(eval(e.index).data);
    
    if (index >= 0 && index < elements.size()) RETURN(elements[index]);

    throw RuntimeError("index " + std::to_string(index) + " out of bounds for length " + std::to_string(elements.size()), Source(e.index.start.line));
  }

  void visitCastExpression(CastExpression& e) override {
    Value value = eval(e.expr);
    
    if (e.type->superset(value.typeOf())) RETURN(value);

    throw RuntimeError("failed to cast type '" + value.typeOf()->toString() + "' to type '" + e.type->toString() + "'", Source(e.start.line));
  }

  void visitPostfixExpression(PostfixExpression& e) override {
    Value& value = getLValue(e.expr);

    if (e.op.type == INCREMENT) {
      if (long long* p = std::get_if<long long>(&value.data)) RETURN(Value{(*p)++});
      else RETURN(Value{std::get<double>(value.data)++});
    }
    else {
      if (long long* p = std::get_if<long long>(&value.data)) RETURN(Value{(*p)--});
      else RETURN(Value{std::get<double>(value.data)--});
    }
  }

  void visitUnaryExpression(UnaryExpression& e) override {
    switch (e.op.type) {
      case PLUS: RETURN(eval(e.expr));
      case MINUS: {
        const Value expr = eval(e.expr);

        RETURN(std::holds_alternative<long long>(expr.data)?
          Value{-std::get<long long>(expr.data)}
          : Value{-std::get<double>(expr.data)});
      }
      case INCREMENT: {
        Value& value = getLValue(e.expr);

        if (long long* p = std::get_if<long long>(&value.data)) ++(*p);
        else ++std::get<double>(value.data);

        RETURN(value);
      }
      case DECREMENT: {
        Value& value = getLValue(e.expr);

        if (long long* p = std::get_if<long long>(&value.data)) --(*p);
        else --std::get<double>(value.data);

        RETURN(value);
      }
      default: RETURN(Value{!std::get<bool>(eval(e.expr).data)});
    }
  }

  void visitBinaryExpression(BinaryExpression& e) override {
    const Value left = eval(e.left);
    const Value right = eval(e.right);

    switch (e.op.type) {
      case COALESCENCE: RETURN(std::holds_alternative<std::monostate>(left.data)? right : left);
      case AND: RETURN(Value{std::get<bool>(left.data) && std::get<bool>(right.data)});
      case OR: RETURN(Value{std::get<bool>(left.data) || std::get<bool>(right.data)});
      case EQUALS: RETURN(Value{left == right});
      case NOT_EQUALS: RETURN(Value{!(left == right)});
      case GREATER: RETURN(MATH_OP(left, >, right));
      case LESS: RETURN(MATH_OP(left, <, right));
      case GREATER_EQUALS: RETURN(MATH_OP(left, >=, right));
      case LESS_EQUALS: RETURN(MATH_OP(left, <=, right));
      case PLUS:
        if (std::holds_alternative<std::shared_ptr<Object>>(left.data)) {
          const std::shared_ptr<String> a = std::static_pointer_cast<String>(std::get<std::shared_ptr<Object>>(left.data));
          const std::shared_ptr<String> b = std::static_pointer_cast<String>(std::get<std::shared_ptr<Object>>(right.data));
          
          RETURN(Value{std::make_shared<String>(a->chars + b->chars)});
        }

        RETURN(MATH_OP(left, +, right));

      case MINUS: RETURN(MATH_OP(left, -, right));
      case STAR: RETURN(MATH_OP(left, *, right));
      case SLASH: {
        if (std::holds_alternative<long long>(left.data)) {
          const long long rightNum = std::get<long long>(right.data);

          if (rightNum != 0) RETURN(Value{std::get<long long>(left.data) / rightNum});

          throw RuntimeError("cannot divide by 0", Source(e.op.line));
        }

        RETURN(Value{std::get<double>(left.data) / std::get<double>(right.data)});
      }
      default: {
        if (std::holds_alternative<long long>(left.data)) {
          const long long rightNum = std::get<long long>(right.data);

          if (rightNum != 0) RETURN(Value{std::get<long long>(left.data) % rightNum});

          throw RuntimeError("cannot divide by 0", Source(e.op.line));
        }

        RETURN(Value{std::fmod(std::get<double>(left.data), std::get<double>(right.data))});
      }
    }
  }

  void visitTernaryExpression(TernaryExpression& e) override {
    RETURN(std::get<bool>(eval(e.condition).data)? eval(e.value) : eval(e._default));
  }

  void visitAssignmentExpression(AssignmentExpression& e) override {
    Value& l_value = getLValue(e.l_value);
    const Value value = eval(e.value);

    switch (e.op.type) {
      case ASSIGN: RETURN(l_value = value);
      case PLUS_EQUALS: {
        if (std::holds_alternative<std::shared_ptr<Object>>(value.data)) {
          const std::shared_ptr<String> a = std::static_pointer_cast<String>(std::get<std::shared_ptr<Object>>(l_value.data));
          const std::shared_ptr<String> b = std::static_pointer_cast<String>(std::get<std::shared_ptr<Object>>(value.data));
            
          RETURN(l_value = Value{std::make_shared<String>(a->chars + b->chars)});
        }

        RETURN(MATH_OP(l_value, +=, value));
      }
      case MINUS_EQUALS: RETURN(MATH_OP(l_value, -=, value));
      case STAR_EQUALS: RETURN(MATH_OP(l_value, *=, value));
      case SLASH_EQUALS: {
        if (long long* p = std::get_if<long long>(&l_value.data)) {
          const long long rightNum = std::get<long long>(value.data);

          if (rightNum != 0) RETURN(Value{*p /= rightNum});

          throw RuntimeError("cannot divide by 0", Source(e.op.line));
        }

        RETURN(Value{std::get<double>(l_value.data) /= std::get<double>(value.data)});
      }
      default: {
        if (long long* ptr = std::get_if<long long>(&l_value.data)) {
          const long long rightNum = std::get<long long>(value.data);

          if (rightNum != 0) RETURN(Value{*ptr %= rightNum});

          throw RuntimeError("cannot divide by 0", Source(e.op.line));
        }

        RETURN(l_value = Value{std::fmod(std::get<double>(l_value.data), std::get<double>(value.data))});
      }
    }
  }
} Interpreter;