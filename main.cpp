#include <any>
#include <fstream>
#include <iostream>
#include <string>
#include "argparse.hpp"
#include "ASTNode.h"
#include "Error.h"
#include "Interpreter.h"
#include "Lexer.h"
#include "Parser.h"
#include "Type.h"
#include "TypeChecker.h"
#include "Value.h"

std::string Source::file = "REPL";

Value interpret(std::string input) {
  Program* AST = Parser.parse(Lexer.tokenize(input));
  TypeChecker.visitProgram(*AST);

  if (!hadError) return std::any_cast<Value>(Interpreter.visitProgram(*AST));
  return {};
}

int main(int argc, char* argv[]) {
  argparse::ArgumentParser program("mango", "v0.1.0a");

  program.add_argument("file")
    .default_value("")
    .help("the file to interpret");

  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error& err) {
    std::cerr << "error: " << err.what() << "\n";
    return 1;
  }

  std::ios::sync_with_stdio(false);
  std::cout << std::showpoint;

  std::string fileName = program.get<std::string>("file");

  if (fileName.empty()) {
    std::string input;

    while (true) {
      std::cout << "> ";
      std::getline(std::cin, input);

      if (std::cin.eof()) break;
      else if (input.empty()) continue;

      auto fallback = TypeChecker.stack;
      Value result = interpret(input);

      if (!hadError) std::cout << result << std::endl;
      else {
        hadError = false;
        TypeChecker.stack = fallback;
      }
    }
  }
  else {
    if (fileName.size() <= 6 || fileName.substr(fileName.size() - 6) != ".mango") {
      std::cerr << "error: file does not end in '.mango'";
      return 1;
    }

    std::ifstream file(fileName);

    if (!file.good()) {
      std::cerr << "error: file does not exist";
      return 1;
    }

    Source::file = fileName;
    std::string str{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    interpret(str);
  }
}