#pragma once

#include <any>
#include <charconv>
#include <iostream>
#include <math.h>
#include <system_error>
#include "ASTNode.h"
#include "Error.h"
#include "Value.h"
#include "Environment.h"

#define MATH_OP(left, op, right) std::holds_alternative<long long>(left.data)? \
  Value{std::get<long long>(left.data) op std::get<long long>(right.data)} : \
  Value{std::get<double>(left.data) op std::get<double>(right.data)}

struct Return {
  Value value;
  
  Return(Value value) : value(value) {}
};

struct : Visitor {
  Environment* globals = new Environment(nullptr);
  Environment* current = globals;

  Value& getLValue(Expression& e) {
    if (VarExpression* varExpr = dynamic_cast<VarExpression*>(&e))
      return current->get(std::string(varExpr->id.lexeme));

    ArrayAccessExpression* arrayAccess = dynamic_cast<ArrayAccessExpression*>(&e);
    std::vector<Value>* elements = &std::static_pointer_cast<Array>
      (std::get<std::shared_ptr<Object>>(std::any_cast<Value>(visit(arrayAccess->array)).data))->elements;
    const long long index = std::get<long long>
      (std::any_cast<Value>(visit(arrayAccess->index)).data);

    if (index < 0 || index >= elements->size())
      throw RuntimeError("index " + std::to_string(index) + " out of bounds for length " + std::to_string(elements->size()), Source((*arrayAccess).index.start.line));

    return (*elements)[index];
  }

  std::any visit(ASTNode& e) override {
    return e.accept(*this);
  }

  std::any visitProgram(Program& p) override {
    try {
      for (int i = 0; i < p.statements.size() - 1; ++i)
        visit(*p.statements[i]);

      const auto result = visit(*p.statements[p.statements.size() - 1]);

      if (dynamic_cast<ExpressionStatement*>(p.statements[p.statements.size() - 1]))
        return result;
    }
    catch (RuntimeError e) {}

    return Value{std::monostate{}};
  }

  std::any visitBlockStatement(BlockStatement& s) {
    Environment* previous = current;
    current = new Environment(current);

    for (Statement* statement : s.statements)
      visit(*statement);

    current = previous;
    return std::any{};
  }

  std::any visitExpressionStatement(ExpressionStatement& s) override {
    return visit(s.expr);
  }

  std::any visitIfStatement(IfStatement& s) {
    if (std::get<bool>(std::any_cast<Value>(visit(s.condition)).data))
      visit(s.thenBranch);
    else if (s.elseBranch)
      visit(*s.elseBranch);

    return std::any{};
  }

  std::any visitPrintStatement(PrintStatement& s) override {
    std::cout << std::any_cast<Value>(visit(s.expr)).toString() << std::endl;
    return std::any{};
  }

  std::any visitReturnStatement(ReturnStatement& s) {
    throw Return(s.value? std::any_cast<Value>(visit(*s.value)) : Value{std::monostate{}});
  }

  std::any visitWhileStatement(WhileStatement& s) {
    while (std::get<bool>(std::any_cast<Value>(visit(s.condition)).data)) {
      visit(s.body);
    }

    return std::any{};
  }

  std::any visitFunctionDeclaration(FunctionDeclaration& s) override {
    current = new Environment(current);
    current->set(std::string(s.id.lexeme), Value{std::make_shared<Function>(s, current)});
    return std::any{};
  }

  std::any visitVarDeclaration(VarDeclaration& s) override {
    current = new Environment(current);
    current->set(
      std::string(s.id.lexeme),
      s.initializer? std::any_cast<Value>(visit(*s.initializer)) : Value{std::monostate{}}
    );
    return std::any{};
  }

  std::any visitConstDeclaration(ConstDeclaration& s) override {
    current = new Environment(current);
    current->set(std::string(s.id.lexeme), std::any_cast<Value>(visit(s.initializer)));
    return std::any{};
  }

  std::any visitLiteralExpression(LiteralExpression& e) override {
    switch (e.value.type) {
      case INT: {
        long long result;
        auto [_, ec] = std::from_chars(e.value.lexeme.data(), e.value.lexeme.data() + e.value.lexeme.size(), result);
        
        if (ec == std::errc::result_out_of_range)
          throw RuntimeError("number " + std::string(e.value.lexeme) + " out of range for type 'int'", Source(e.value.line));

        return Value{result};
      }
      case FLOAT: {
        double result;
        auto [_, ec] = std::from_chars(e.value.lexeme.data(), e.value.lexeme.data() + e.value.lexeme.size(), result);
        
        if (ec == std::errc::result_out_of_range)
          throw RuntimeError("number " + std::string(e.value.lexeme) + " out of range for type 'float'", Source(e.value.line));

        return Value{result};
      }
      case STRING: return Value{std::make_shared<String>(e.value.lexeme)};
      case CHAR: return Value{e.value.lexeme[0]};
      case TRUE: return Value{true};
      case FALSE: return Value{false};
      default: return Value{std::monostate{}};
    }
  }

  std::any visitArrayLiteralExpression(ArrayLiteralExpression& e) override {
    std::vector<Value> elements;

    for (Expression* element : e.elements)
      elements.push_back(std::any_cast<Value>(visit(*element)));

    return Value{std::make_shared<Array>(elements)};
  }

  std::any visitVarExpression(VarExpression& e) override {
    return current->get(std::string(e.id.lexeme));
  }

  std::any visitCallExpression(CallExpression& e) {
    Value callee = std::any_cast<Value>(visit(e.callee));
    std::shared_ptr<Function> function = std::static_pointer_cast<Function>(std::get<std::shared_ptr<Object>>(callee.data));

    Environment* env = new Environment(function->closure);

    for (int i = 0; i < function->declaration.args.size(); ++i)
      env->set(std::string(function->declaration.args[i].id.lexeme), std::any_cast<Value>(visit(*e.args[i])));

    Environment* previous = current;
    current = env;

    try {
      for (Statement* statement : function->declaration.body.statements)
        visit(*statement);
    }
    catch(Return r) {
      current = previous;
      return r.value;
    }

    current = previous;
    return Value{std::monostate{}};
  }

  std::any visitArrayAccessExpression(ArrayAccessExpression& e) override {
    const std::vector<Value> elements = std::static_pointer_cast<Array>
      (std::get<std::shared_ptr<Object>>(std::any_cast<Value>(visit(e.array)).data))->elements;
    const long long index = std::get<long long>
      (std::any_cast<Value>(visit(e.index)).data);
    
    if (index >= 0 && index < elements.size()) return elements[index];

    throw RuntimeError("index " + std::to_string(index) + " out of bounds for length " + std::to_string(elements.size()), Source(e.index.start.line));
  }

  std::any visitCastExpression(CastExpression& e) override {
    Value value = std::any_cast<Value>(visit(e.expr));
    
    if (e.type->superset(value.typeOf())) return value;

    throw RuntimeError("failed to cast type '" + value.typeOf()->toString() + "' to type '" + e.type->toString() + "'", Source(e.start.line));
  }

  std::any visitPostfixExpression(PostfixExpression& e) override {
    Value& value = getLValue(e.expr);

    if (e.op.type == INCREMENT) {
      if (long long* p = std::get_if<long long>(&value.data)) return Value{(*p)++};
      else return Value{std::get<double>(value.data)++};
    }
    else {
      if (long long* p = std::get_if<long long>(&value.data)) return Value{(*p)--};
      else return Value{std::get<double>(value.data)--};
    }
  }

  std::any visitUnaryExpression(UnaryExpression& e) override {
    switch (e.op.type) {
      case PLUS: return std::any_cast<Value>(visit(e.expr));
      case MINUS: {
        const Value expr = std::any_cast<Value>(visit(e.expr));

        return std::holds_alternative<long long>(expr.data)?
          Value{-std::get<long long>(expr.data)}
          : Value{-std::get<double>(expr.data)};
      }
      case INCREMENT: {
        Value& value = getLValue(e.expr);

        if (long long* p = std::get_if<long long>(&value.data)) ++(*p);
        else ++std::get<double>(value.data);

        return value;
      }
      case DECREMENT: {
        Value& value = getLValue(e.expr);

        if (long long* p = std::get_if<long long>(&value.data)) --(*p);
        else --std::get<double>(value.data);

        return value;
      }
      default: return Value{!std::get<bool>(std::any_cast<Value>(visit(e.expr)).data)};
    }
  }

  std::any visitBinaryExpression(BinaryExpression& e) override {
    const Value left = std::any_cast<Value>(visit(e.left));
    const Value right = std::any_cast<Value>(visit(e.right));

    switch (e.op.type) {
      case COALESCENCE: return std::holds_alternative<std::monostate>(left.data)? right : left;
      case AND: return Value{std::get<bool>(left.data) && std::get<bool>(right.data)};
      case OR: return Value{std::get<bool>(left.data) || std::get<bool>(right.data)};
      case EQUALS: return Value{left == right};
      case NOT_EQUALS: return Value{!(left == right)};
      case GREATER: return MATH_OP(left, >, right);
      case LESS: return MATH_OP(left, <, right);
      case GREATER_EQUALS: return MATH_OP(left, >=, right);
      case LESS_EQUALS: return MATH_OP(left, <=, right);
      case PLUS:
        if (std::holds_alternative<std::shared_ptr<Object>>(left.data)) {
          const std::shared_ptr<String> a = std::static_pointer_cast<String>(std::get<std::shared_ptr<Object>>(left.data));
          const std::shared_ptr<String> b = std::static_pointer_cast<String>(std::get<std::shared_ptr<Object>>(right.data));
          
          return Value{std::make_shared<String>(a->chars + b->chars)};
        }

        return MATH_OP(left, +, right);

      case MINUS: return MATH_OP(left, -, right);
      case STAR: return MATH_OP(left, *, right);
      case SLASH: {
        if (std::holds_alternative<long long>(left.data)) {
          const long long rightNum = std::get<long long>(right.data);

          if (rightNum != 0) return Value{std::get<long long>(left.data) / rightNum};

          throw RuntimeError("cannot divide by 0", Source(e.op.line));
        }

        return Value{std::get<double>(left.data) / std::get<double>(right.data)};
      }
      default: {
        if (std::holds_alternative<long long>(left.data)) {
          const long long rightNum = std::get<long long>(right.data);

          if (rightNum != 0) return Value{std::get<long long>(left.data) % rightNum};

          throw RuntimeError("cannot divide by 0", Source(e.op.line));
        }

        return Value{std::fmod(std::get<double>(left.data), std::get<double>(right.data))};
      }
    }
  }

  std::any visitTernaryExpression(TernaryExpression& e) override {
    return std::get<bool>(std::any_cast<Value>(visit(e.condition)).data)?
      visit(e.value) : visit(e._default);
  }

  std::any visitAssignmentExpression(AssignmentExpression& e) override {
    Value& l_value = getLValue(e.l_value);
    const Value value = std::any_cast<Value>(visit(e.value));

    switch (e.op.type) {
      case ASSIGN: return l_value = value;
      case PLUS_EQUALS: {
        if (std::holds_alternative<std::shared_ptr<Object>>(value.data)) {
          const std::shared_ptr<String> a = std::static_pointer_cast<String>(std::get<std::shared_ptr<Object>>(l_value.data));
          const std::shared_ptr<String> b = std::static_pointer_cast<String>(std::get<std::shared_ptr<Object>>(value.data));
            
          return l_value = Value{std::make_shared<String>(a->chars + b->chars)};
        }

        return MATH_OP(l_value, +=, value);
      }
      case MINUS_EQUALS: return MATH_OP(l_value, -=, value);
      case STAR_EQUALS: return MATH_OP(l_value, *=, value);
      case SLASH_EQUALS: {
        if (long long* p = std::get_if<long long>(&l_value.data)) {
          const long long rightNum = std::get<long long>(value.data);

          if (rightNum != 0) return Value{*p /= rightNum};

          throw RuntimeError("cannot divide by 0", Source(e.op.line));
        }

        return Value{std::get<double>(l_value.data) /= std::get<double>(value.data)};
      }
      default: {
        if (long long* ptr = std::get_if<long long>(&l_value.data)) {
          const long long rightNum = std::get<long long>(value.data);

          if (rightNum != 0) return Value{*ptr %= rightNum};

          throw RuntimeError("cannot divide by 0", Source(e.op.line));
        }

        return l_value = Value{std::fmod(std::get<double>(l_value.data), std::get<double>(value.data))};
      }
    }
  }
} Interpreter;