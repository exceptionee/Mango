#include <iostream>
#include <fstream>
#include <memory>
#include "Lexer.h"
#include "Parser.h"
#include "ASTNode.h"
#include "Visitor.h"
#include "Interpreter.h"
#include "Value.cpp"
using namespace std;

Value interpret(string input) {
  Program* AST = Parser.parse(Lexer.tokenize(input));

  if (!hadError) return any_cast<Value>(Interpreter.visitProgram(*AST));
  return {};
}

int main(int argc, char* argv[]) {
  std::ios::sync_with_stdio(false);
  cout << showpoint;

  if (argc == 1) {
    string input;

    while (true) {
      cout << "> ";
      std::getline(cin, input);

      if (cin.eof()) break;
      else if (input.empty()) continue;

      Value result = interpret(input);

      if (!hadError) cout << result << endl;
      else hadError = false;
    }
  }
  else {
    for (int i = 1; i < argc; ++i) {
      ifstream file { argv[i] };
      string str { istreambuf_iterator<char>(file), istreambuf_iterator<char>() };
      interpret(str);
    }
  }
}