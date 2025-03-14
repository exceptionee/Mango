#include <unordered_map>
#include <vector>
#include "Error.h"
#include "Token.h"

struct {
  std::string_view source;
  int start;
  int current;
  int line;
  std::vector<Token> tokens{};
  std::unordered_map<std::string_view, TokenType> keywords = {
    {"var", VAR},
    {"const", CONST},
    {"any", ANY_TYPE},
    {"int", INT_TYPE},
    {"float", FLOAT_TYPE},
    {"string", STRING_TYPE},
    {"char", CHAR_TYPE},
    {"bool", BOOL_TYPE},
    {"true", TRUE},
    {"false", FALSE},
    {"null", _NULL},
    {"as", AS},
    {"if", IF},
    {"else", ELSE},
    {"while", WHILE},
    {"print", PRINT}
  };

  /**
   * Scans the input stream for the next valid token and appends it to `tokens`.
   */
  inline void scanToken() {
    const char c = advance();

    switch (c) {
      case ' ':
      case '\r':
      case '\t':
        return;

      case '\n':
        ++line;
        return;

      case '(': return tokens.push_back(Token(LEFT_PAREN, "(", line));
      case ')': return tokens.push_back(Token(RIGHT_PAREN, ")", line));
      case '{': return tokens.push_back(Token(LEFT_BRACE, "{", line));
      case '}': return tokens.push_back(Token(RIGHT_BRACE, "}", line));
      case '[': return tokens.push_back(Token(LEFT_BRACKET, "[", line));
      case ']': return tokens.push_back(Token(RIGHT_BRACKET, "]", line));
      case ',': return tokens.push_back(Token(COMMA, ",", line));
      case '.': return tokens.push_back(Token(DOT, ".", line));
      case ';': return tokens.push_back(Token(SEMI, ";", line));
      case ':': return tokens.push_back(Token(COLON, ":", line));

      case '+':
        return tokens.push_back(match('+')? Token(INCREMENT, "++", line) :
          match('=')? Token(PLUS_EQUALS, "+=", line) : Token(PLUS, "+", line));
      case '-':
        return tokens.push_back(match('-')? Token(DECREMENT, "--", line) :
          match('=')? Token(MINUS_EQUALS, "-=", line) : Token(MINUS, "-", line));
      case '*':
        return tokens.push_back(match('=')? Token(STAR_EQUALS, "*=", line) : Token(STAR, "*", line));

      case '/':
        if (match('=')) return tokens.push_back(Token(SLASH_EQUALS, "/=", line));
        else if (match('/')) { // matches line comments
          while (current < source.size() && peek() != '\n')
            ++current;

          return;
        }
        else if (match('*')) { // matches multiline comments
          if (advance() == '\n') line++;

          while (current < source.size()) {
            const char c = advance();

            if (c == '\n') line++;
            else if (c == '/' && source[current - 2] == '*') return;
          }

          SyntaxError("unterminated comment", Source(line));
          return;
        }

        return tokens.push_back(Token(SLASH, "/", line));
      
      case '%':
        return tokens.push_back(match('=')? Token(MODULO_EQUALS, "%=", line) : Token(MODULO, "%", line));
      case '=':
        return tokens.push_back(match('=')? Token(EQUALS, "==", line) : Token(ASSIGN, "=", line));
      case '!':
        return tokens.push_back(match('=')? Token(NOT_EQUALS, "!=", line) : Token(NOT, "!", line));
      case '>':
        return tokens.push_back(match('=')? Token(GREATER_EQUALS, ">=", line) : Token(GREATER, ">", line));
      case '<':
        return tokens.push_back(match('=')? Token(LESS_EQUALS, "<=", line) : Token(LESS, "<", line));
      case '|':
        return tokens.push_back(match('|')? Token(OR, "||", line) : Token(UNION, "|", line));
      case '&':
        if (match('&')) return tokens.push_back(Token(AND, "&&", line));
        SyntaxError("unrecognized token '&'", Source(line));
        return;
      case '?':
        return tokens.push_back(match('?')? Token(COALESCENCE, "??", line) : Token(TERNARY, "?", line));

      case '"': // handles strings
        while (current < source.size() && peek() != '\n')
          if (advance() == '"') 
            return tokens.push_back(Token(STRING, source.substr(start + 1, current - start - 2), line));

        SyntaxError("unterminated string", Source(line));
        return;

      default:
        if (isdigit(c)) { // handles numbers
          while (isdigit(peek()))
            ++current;

          if (!match('.'))
            return tokens.push_back(Token(INT, source.substr(start, current - start), line));

          while (isdigit(peek()))
            ++current;

          return tokens.push_back(Token(FLOAT, source.substr(start, current - start), line));
        }
        else if (isalpha(c) || c == '_') { // handles identifiers and keywords
          while (isalnum(peek()) || peek() == '_')
            ++current;

          auto it = keywords.find(source.substr(start, current - start));

          return tokens.push_back(Token(it == keywords.end()? ID : it->second,
            source.substr(start, current - start), line));
        }

        SyntaxError("unrecognized token '" + std::string(1, c) + "'", Source(line));
    }
  }

  inline char advance() {
    return source[current++];
  }

  inline bool match(char expected) {
    if (peek() != expected)
      return false;

    ++current;
    return true;
  }

  inline char peek() {
    return source[current];
  }

  inline char peekNext() {
    return source[current + 1];
  }

  std::vector<Token> tokenize(std::string_view source) {
    this->source = source;
    start = 0;
    current = 0;
    line = 1;
    tokens = {};

    while (current < source.size()) {
      start = current;
      scanToken();
    }

    tokens.push_back(Token(_EOF, "", line));
    return tokens;
  }
} Lexer;