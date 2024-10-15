#pragma once

#include <string>
#include <iostream>
#include <ostream>

struct Source {
  const std::string file;
  const int line;

  Source(std::string file, int line) : file(file), line(line) {}

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
    std::cout << type << ": " << message << "\n\t" << location;
  }
};

struct SyntaxError : Error {
  SyntaxError(std::string message, Source location)
    : Error("SyntaxError", message, location) {}
};

struct RuntimeError : Error {
  RuntimeError(std::string message, Source location)
    : Error("RuntimeError", message, location) {}
};