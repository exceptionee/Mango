#include <vector>
#include <typeinfo>
#include "Token.h"
#include "ASTNode.h"
#include "Error.h"

struct {
  std::vector<Token> tokens;
  int current;

  Program* parse(std::vector<Token> tokens) {
    this->tokens = std::move(tokens);
    current = 0;
    
    return program();
  }

  Program* program() {
    std::vector<Statement*> statements;

    while (current < tokens.size() - 1)
      statements.push_back(declaration());

    return new Program(statements);
  }

  Statement* declaration() {
    try {
      if (match(VAR)) {
        Token id = consume(ID, "expected identifier");
        Expression* initializer = match(ASSIGN)? expression() : nullptr;
        consume(SEMI, "expected ';'");

        return new VarDeclarationStatement(id, initializer);
      }
      else if (match(CONST)) {
        Token id = consume(ID, "expected identifier");
        consume(ASSIGN, "'const' declarations must have an initializer");
        Expression* initializer = expression();
        consume(SEMI, "expected ';'");

        return new ConstDeclarationStatement(id, *initializer);
      }

      return statement();
    }
    catch (SyntaxError e) {
      synchronize();
      return nullptr;
    }
  }

  Statement* statement() {
    if (match(PRINT)) {
      Expression* expr = expression();
      consume(SEMI, "expected ';'");
      return new PrintStatement(*expr);
    }
    else if (match(LEFT_BRACE)) {
      std::vector<Statement*> statements;

      while (peek().type != RIGHT_BRACE && current < tokens.size() - 1)
        statements.push_back(declaration());

      consume(RIGHT_BRACE, "expected '}' after block");
      return new BlockStatement(statements);
    }

    Expression* expr = expression();
    consume(SEMI, "expected ';'");
    return new ExpressionStatement(*expr);
  }

  Expression* expression() {
    return assignment();
  }

  Expression* assignment() {
    Expression* expr = ternary();

    if (match(ASSIGN) || match(PLUS_EQUALS) || match(MINUS_EQUALS) || match(STAR_EQUALS) || match(SLASH_EQUALS) || match(MODULO_EQUALS)) {
      const Token op = previous();
      return new AssignmentExpression(*expr, op, *assignment());
    }

    return expr;
  }

  Expression* ternary() {
    Expression* expr = coalescing();

    if (match(TERNARY)) {
      Expression* value = ternary();
      consume(COLON, "expected ':'");
      Expression* _default = ternary();

      return new TernaryExpression(*expr, *value, *_default);
    }

    return expr;
  }

  Expression* coalescing() {
    Expression* expr = logical();

    while (match(COALESCENCE)) {
      const Token op = previous();
      expr = new BinaryExpression(*expr, op, *logical());
    }

    return expr;
  }

  Expression* logical() {
    Expression* expr = equality();

    while (match(AND) || match(OR)) {
      const Token op = previous();
      expr = new BinaryExpression(*expr, op, *equality());
    }

    return expr;
  }

  Expression* equality() {
    Expression* expr = comparison();

    while (match(EQUALS) || match(NOT_EQUALS)) {
      const Token op = previous();
      expr = new BinaryExpression(*expr, op, *comparison());
    }

    return expr;
  }

  Expression* comparison() {
    Expression* expr = term();

    while (match(GREATER) || match(LESS) || match(GREATER_EQUALS) || match(LESS_EQUALS)) {
      const Token op = previous();
      expr = new BinaryExpression(*expr, op, *term());
    }

    return expr;
  }

  Expression* term() {
    Expression* expr = factor();

    while (match(PLUS) || match(MINUS)) {
      const Token op = previous();
      expr = new BinaryExpression(*expr, op, *factor());
    }

    return expr;
  }

  Expression* factor() {
    Expression* expr = unary();

    while (match(STAR) || match(SLASH) || match(MODULO)) {
      const Token op = previous();
      expr = new BinaryExpression(*expr, op, *unary());
    }

    return expr;
  }

  Expression* unary() {
    if (match(PLUS) || match(MINUS) || match(INCREMENT) || match(DECREMENT) || match(NOT)) {
      const Token op = previous();
      return new UnaryExpression(op, *unary());
    }

    return postfix();
  }

  Expression* postfix() {
    Expression* expr = access();

    if (match(INCREMENT) || match(DECREMENT))
      return new PostfixExpression(*expr, previous());

    return expr;
  }

  Expression* access() {
    Expression* expr = primary();

    while (match(LEFT_BRACKET)) {
      Expression* index = expression();
      const Token bracket = consume(RIGHT_BRACKET, "expected ']' after index");

      expr = new ArrayAccessExpression(*expr, bracket, *index);
    }

    return expr;
  }

  Expression* primary() {
    if (match(LEFT_PAREN)) {
      Expression* expr = expression();
      consume(RIGHT_PAREN, "expected ')' after expression");

      return new ParenExpression(*expr);
    }
    else if (match(LEFT_BRACKET)) {
      std::vector<Expression*> elements;

      if (!match(RIGHT_BRACKET)) {
        do {
          elements.push_back(expression());
        } while (match(COMMA));

        consume(RIGHT_BRACKET, "expected ']' after expression");
      }

      return new ArrayLiteralExpression(elements);
    }

    switch (peek().type) {
      case INT:
      case FLOAT:
      case STRING:
      case TRUE:
      case FALSE:
      case _NULL:
        return new LiteralExpression(advance());
      case ID: return new VarExpression(advance());
    }

    throw SyntaxError("expected expression", Source("REPL", peek().line));
  }

  inline Token advance() {
    return tokens[current++];
  }

  inline bool match(TokenType type) {
    if (peek().type != type)
      return false;

    ++current;
    return true;
  }

  inline Token consume(TokenType type, std::string message) {
    if (peek().type == type) return advance();
    throw SyntaxError(message, Source("REPL", peek().line));
  }

  inline Token peek() {
    return tokens[current];
  }

  inline Token previous() {
    return tokens[current - 1];
  }

  inline void synchronize() {
    advance();

    while (current < tokens.size() - 1) {
      if (previous().type == SEMI) return;

      switch (peek().type) {
        case VAR:
        case PRINT:
          return;
      }

      advance();
    }
  }
} Parser;