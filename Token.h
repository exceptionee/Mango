#pragma once

enum TokenType {
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  LEFT_BRACKET,
  RIGHT_BRACKET,
  ARROW,
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
  FUNCTION,
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
  FOR,
  IF,
  ELSE,
  PRINT,
  RETURN,
  WHILE,
  _EOF
};

struct Token {
  TokenType type;
  std::string lexeme;
  int line;

  Token(TokenType type, std::string lexeme, int line)
    : type(type), lexeme(lexeme), line(line) {}
};