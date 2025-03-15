#pragma once

#include <iostream>
#include <string>

struct Source {
  static std::string file;

  const int line;

  Source(int line) : line(line) {}

  friend std::ostream& operator<<(std::ostream& os, Source& s) {
    return (os << "at " << s.file << ":" << s.line << '\n');
  }
};

bool hadError = false;

struct Error {
  const std::string message;
  const Source location;

  Error(std::string type, std::string message, Source location)
    : message(message), location(location) {

    hadError = true;
    std::cerr << type << ": " << message << "\n\t" << location;
  }
};

struct SyntaxError : Error {
  SyntaxError(std::string message, Source location)
    : Error("SyntaxError", message, location) {}
};

struct TypeError : Error {
  TypeError(std::string message, Source location)
    : Error("TypeError", message, location) {}
};

struct ReferenceError : Error {
  ReferenceError(std::string message, Source location)
    : Error("ReferenceError", message, location) {}
};

struct RuntimeError : Error {
  RuntimeError(std::string message, Source location)
    : Error("RuntimeError", message, location) {}
};