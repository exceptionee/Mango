#pragma once

#include <ostream>

enum TokenType {
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  LEFT_BRACKET,
  RIGHT_BRACKET,
  COMMA,
  DOT,
  SEMI,
  COLON,
  PLUS,
  PLUS_EQUALS,
  INCREMENT,
  MINUS,
  MINUS_EQUALS,
  DECREMENT,
  STAR,
  STAR_EQUALS,
  SLASH,
  SLASH_EQUALS,
  MODULO,
  MODULO_EQUALS,
  UNION,
  OR,
  AND,
  TERNARY,
  COALESCENCE,
  ASSIGN,
  EQUALS,
  NOT,
  NOT_EQUALS,
  GREATER,
  LESS,
  GREATER_EQUALS,
  LESS_EQUALS,
  ID,
  VAR,
  CONST,
  ANY_TYPE,
  INT_TYPE,
  FLOAT_TYPE,
  STRING_TYPE,
  CHAR_TYPE,
  BOOL_TYPE,
  INT,
  FLOAT,
  STRING,
  CHAR,
  TRUE,
  FALSE,
  _NULL,
  AS,
  PRINT,
  _EOF
};

struct Token {
  TokenType type;
  std::string_view lexeme;
  int line;

  Token(TokenType type, std::string_view lexeme, int line)
    : type(type), lexeme(lexeme), line(line) {}

  Token& operator=(const Token&) = default;

  Token(const Token& token)
    : type(token.type), lexeme(token.lexeme), line(token.line) {}

  Token(Token&& token)
    : type(token.type), lexeme(token.lexeme), line(token.line) {}

  friend std::ostream& operator<<(std::ostream& os, Token token) {
    return (os << "['" << token.lexeme << "', " << token.line << ']');
  }
};