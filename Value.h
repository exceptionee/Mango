#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>
#include "ASTNode.h"

struct Environment;

enum ObjectType {
  OBJECT_FUNCTION,
  OBJECT_STRING,
  OBJECT_ARRAY
};

struct Object {
  ObjectType type;

  Object(ObjectType type) : type(type) {}
  virtual ~Object() = default;
  virtual bool equals(Object& o) = 0;
  virtual std::string toString() = 0;
};

struct Value {
  std::variant<long long, double, char, bool, std::shared_ptr<Object>, std::monostate> data;

  bool operator==(const Value& v) const {
    if (data.index() != v.data.index()) return false;

    switch (data.index()) {
      case 0: return std::get<long long>(data) == std::get<long long>(v.data);
      case 1: return std::get<double>(data) == std::get<double>(v.data);
      case 3: return std::get<bool>(data) == std::get<bool>(v.data);
      case 4: return std::get<std::shared_ptr<Object>>(data)
        ->equals(*std::get<std::shared_ptr<Object>>(v.data));
      default: return true; // when both values are null
    }
  }

  std::string toString() {
    switch (data.index()) {
      case 0: return "\e[33m" + std::to_string(std::get<long long>(data)) + "\e[0m";
      case 1: return "\e[33m" + std::to_string(std::get<double>(data)) + "\e[0m";
      case 3: return std::get<bool>(data)? "\e[31mtrue\e[0m" : "\e[31mfalse\e[0m";
      case 4: return std::get<std::shared_ptr<Object>>(data)->toString();
      default: return "\e[38;5;250mnull\e[0m";
    }
  }
};

struct Function : Object {
  FunctionDeclaration& declaration;
  Environment* closure;

  Function(FunctionDeclaration& declaration, Environment* closure)
    : Object{ObjectType::OBJECT_FUNCTION}, declaration(declaration), closure(closure) {}

  bool equals(Object& o) override {
    return this == &o;
  }
  
  std::string toString() override {
    return "\e[34m<function: " + std::string(declaration.id.lexeme) + ">\e[0m";
  }
};

struct String : Object {
  std::string chars;

  String(std::string const& chars) : Object {OBJECT_STRING}, chars(chars) {}

  bool equals(Object& o) override {
    if (o.type == OBJECT_STRING)
      return chars == ((String*) &o)->chars;
    return false;
  }

  std::string toString() override {
    return "\e[32m" + chars + "\e[0m";
  }
};

struct Array : Object {
  std::vector<Value> elements;

  Array(std::vector<Value> const& elements)
    : Object{ObjectType::OBJECT_ARRAY}, elements(elements) {}

  bool equals(Object& o) override {
    return this == &o;
  }
  
  std::string toString() override {
    std::string result = "[";

    for (int i = 0; i < elements.size(); ++i) {
      result += elements[i].toString();
      if (i != elements.size() - 1) result += ", ";
    }

    return (result + "]");
  }
};