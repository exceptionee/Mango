#include <utility>
#include <vector>
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

    while (current < tokens.size() - 1) {
      Statement* statement = declaration();

      if (statement) statements.push_back(statement);
    }

    return new Program(statements);
  }

  Type* type() {
    Type* t = arrayType();

    if (match(UNION)) {
      auto u = new UnionType({ t, arrayType() });

      while (match(UNION))
        u->add(arrayType());

      if (u->types.size() == 1) return u->types[0];
      return u;
    }

    return t;
  }

  Type* arrayType() {
    Type* t = primaryType();

    while (match(LEFT_BRACKET)) {
      consume(RIGHT_BRACKET, "expected ']'");
      t = new ArrayType(t);
    }

    return t;
  }

  Type* primaryType() {
    if (match(LEFT_PAREN)) {
      if (match(RIGHT_PAREN)) {
        consume(ARROW, "expected '->'");
        return new FunctionType({}, type());
      }

      Type* t = type();

      if (match(RIGHT_PAREN)) {
        if (match(ARROW)) return new FunctionType({ t }, type());
        return t;
      }

      std::vector<Type*> args = { t };

      while (match(COMMA))
        args.push_back(type());

      consume(RIGHT_PAREN, "expected ')'");
      consume(ARROW, "expected '->'");
      return new FunctionType(args, type());
    }

    Token t = advance();

    switch (t.type) {
      case ANY_TYPE: return ANY_T;
      case INT_TYPE: return INT_T;
      case FLOAT_TYPE: return FLOAT_T;
      case STRING_TYPE: return STRING_T;
      case CHAR_TYPE: return CHAR_T;
      case BOOL_TYPE: return BOOL_T;
      case _NULL: return NULL_T;
    }

    throw SyntaxError("expected type", Source(t.line));
  }

  Statement* declaration() {
    try {
      if (match(FUNCTION)) return functionDeclaration();
      else if (match(VAR)) return varDeclaration();
      else if (match(CONST)) return constDeclaration();

      return statement();
    }
    catch (SyntaxError e) {
      synchronize();
      return nullptr;
    }
  }

  Statement* functionDeclaration() {
    Token id = consume(ID, "expected identifier");
    consume(LEFT_PAREN, "expected '('");

    std::vector<Argument> args;
    
    if (peek().type != RIGHT_PAREN) {
      do {
        args.push_back(argument());
      } while (match(COMMA));
    }
    consume(RIGHT_PAREN, "expect ')' after parameters");

    Type* returnType = match(COLON)? type() : ANY_T;

    BlockStatement* body = block();

    return new FunctionDeclaration(id, args, returnType, *body);
  }

  Argument argument() {
    Token id = consume(ID, "expected identifier");
    Type* t = match(COLON)? type() : ANY_T;

    return Argument(id, t);
  }

  Statement* varDeclaration() {
    Token id = consume(ID, "expected identifier");
    Type* t = match(COLON)? type() : nullptr;
    Expression* initializer = match(ASSIGN)? expression() : nullptr;
    consume(SEMI, "expected ';'");

    return new VarDeclaration(id, t, initializer);
  }

  Statement* constDeclaration() {
    Token id = consume(ID, "expected identifier");
    Type* t = match(COLON)? type() : nullptr;
    consume(ASSIGN, "'const' declarations must have an initializer");
    Expression* initializer = expression();
    consume(SEMI, "expected ';'");

    return new ConstDeclaration(id, t, *initializer);
  }

  Statement* statement() {
    if (match(FOR)) {
      consume(LEFT_PAREN, "expected '(' after 'for'");
      Statement* initializer = match(SEMI)? nullptr :
        match(VAR)? varDeclaration() :
        match(CONST)? constDeclaration() : expressionStatement();

      Expression* condition = peek().type == SEMI?
        new LiteralExpression(Token(TRUE, "true", 0)) : expression();
      consume(SEMI, "expected ';' after condition");

      Expression* increment = peek().type == RIGHT_PAREN? nullptr : expression();
      consume(RIGHT_PAREN, "expected ')' after clauses");

      Statement* body = statement();

      if (increment)
        body = new BlockStatement({ body, new ExpressionStatement(*increment) });

      body = new WhileStatement(*condition, *body);
      
      if (initializer)
        body = new BlockStatement({ initializer, body });

      return body;
    }
    else if (match(IF)) {
      consume(LEFT_PAREN, "expected '(' after 'if'");
      Expression* condition = expression();
      consume(RIGHT_PAREN, "expected ')' after condition");

      Statement* thenBranch = statement();
      Statement* elseBranch = match(ELSE)? statement() : nullptr;

      return new IfStatement(*condition, *thenBranch, elseBranch);
    }
    else if (match(PRINT)) {
      Expression* expr = expression();
      consume(SEMI, "expected ';'");
      return new PrintStatement(*expr);
    }
    else if (match(RETURN)) {
      Expression* value = peek().type == SEMI? nullptr : expression();
      consume(SEMI, "expect ';' after return value");
      return new ReturnStatement(value);
    }
    else if (match(WHILE)) {
      consume(LEFT_PAREN, "expected '(' after 'while'");
      Expression* condition = expression();
      consume(RIGHT_PAREN, "expected ')' after condition");
      Statement* body = statement();
      
      return new WhileStatement(*condition, *body);
    }
    else if (peek().type == LEFT_BRACE) return block();

    return expressionStatement();
  }

  BlockStatement* block() {
    consume(LEFT_BRACE, "expected '{'");
    std::vector<Statement*> statements;

    while (peek().type != RIGHT_BRACE && current < tokens.size() - 1)
      statements.push_back(declaration());

    consume(RIGHT_BRACE, "expected '}'");
    return new BlockStatement(statements);
  }

  Statement* expressionStatement() {
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
    Expression* expr = cast();

    if (match(INCREMENT) || match(DECREMENT))
      return new PostfixExpression(*expr, previous());

    return expr;
  }

  Expression* cast() {
    Expression* expr = access();

    while (match(AS))
      expr = new CastExpression(*expr, type());

    return expr;
  }

  Expression* access() {
    Expression* expr = call();

    while (match(LEFT_BRACKET)) {
      Expression* index = expression();
      const Token bracket = consume(RIGHT_BRACKET, "expected ']' after index");

      expr = new ArrayAccessExpression(*expr, *index);
    }

    return expr;
  }

  Expression* call() {
    Expression* expr = primary();

    while (match(LEFT_PAREN)) {
      std::vector<Expression*> args = {};

      if (peek().type != RIGHT_PAREN) {
        do {
          args.push_back(expression());
        } while (match(COMMA));
      }

      Token rightParen = consume(RIGHT_PAREN, "expected ')' after arguments");

      expr = new CallExpression(*expr, args, rightParen);
    }

    return expr;
  }

  Expression* primary() {
    if (match(LEFT_PAREN)) {
      Expression* expr = expression();
      consume(RIGHT_PAREN, "expected ')' after expression");
      return expr;
    }
    else if (match(LEFT_BRACKET)) {
      const Token lb = previous();
      std::vector<Expression*> elements;

      if (!match(RIGHT_BRACKET)) {
        do {
          elements.push_back(expression());
        } while (match(COMMA));

        consume(RIGHT_BRACKET, "expected ']' after expression");
      }

      return new ArrayLiteralExpression(lb, elements);
    }

    switch (peek().type) {
      case INT:
      case FLOAT:
      case STRING:
      case CHAR:
      case TRUE:
      case FALSE:
      case _NULL:
        return new LiteralExpression(advance());
      case ID: return new VarExpression(advance());
    }

    throw SyntaxError("expected expression", Source(peek().line));
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
    throw SyntaxError(message, Source(peek().line));
  }

  inline Token peek() {
    return tokens[current];
  }

  inline Token previous() {
    return tokens[current - 1];
  }

  /**
   * Skips tokens until it finds a token that could feasibly start a new statement.
   */
  inline void synchronize() {
    advance();

    while (current < tokens.size() - 1) {
      if (previous().type == SEMI) return;

      switch (peek().type) {
        case FUNCTION:
        case VAR:
        case CONST:
        case FOR:
        case IF:
        case WHILE:
        case PRINT:
        case RETURN:
          return;
      }

      advance();
    }
  }
} Parser;