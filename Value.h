#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>
#include "ASTNode.h"
#include "Type.h"

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
  virtual Type* typeOf() = 0;
  virtual std::string toString() = 0;
};

struct Value {
  std::variant<long long, double, char, bool, std::shared_ptr<Object>, std::monostate> data;

  bool operator==(const Value& v) const {
    if (data.index() != v.data.index()) return false;

    switch (data.index()) {
      case 0: return std::get<long long>(data) == std::get<long long>(v.data);
      case 1: return std::get<double>(data) == std::get<double>(v.data);
      case 2: return std::get<char>(data) == std::get<char>(v.data);
      case 3: return std::get<bool>(data) == std::get<bool>(v.data);
      case 4: return std::get<std::shared_ptr<Object>>(data)
        ->equals(*std::get<std::shared_ptr<Object>>(v.data));
      default: return true; // when both values are null
    }
  }

  Type* typeOf() {
    switch (data.index()) {
      case 0: return INT_T;
      case 1: return FLOAT_T;
      case 2: return CHAR_T;
      case 3: return BOOL_T;
      case 4: return std::get<std::shared_ptr<Object>>(data)->typeOf();
      default: return NULL_T;
    }
  }

  std::string toString() {
    switch (data.index()) {
      case 0: return "\e[33m" + std::to_string(std::get<long long>(data)) + "\e[0m";
      case 1: return "\e[33m" + std::to_string(std::get<double>(data)) + "\e[0m";
      case 2: return "\e[33m" + std::string(1, std::get<char>(data)) + "\e[0m";
      case 3: return std::get<bool>(data)? "\e[31mtrue\e[0m" : "\e[31mfalse\e[0m";
      case 4: return std::get<std::shared_ptr<Object>>(data)->toString();
      default: return "\e[38;5;250mnull\e[0m";
    }
  }
};

struct Function : Object {
  FunctionDeclaration& declaration;
  std::shared_ptr<Environment> closure;

  Function(FunctionDeclaration& declaration, std::shared_ptr<Environment> closure)
    : Object{ObjectType::OBJECT_FUNCTION}, declaration(declaration), closure(closure) {}

  bool equals(Object& o) override {
    return this == &o;
  }

  Type* typeOf() override {
    std::vector<Type*> args;

    for (Argument arg : declaration.args)
      args.push_back(arg.type);

    return new FunctionType(args, declaration.returnType);
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

  Type* typeOf() override {
    return STRING_T;
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

  Type* typeOf() override {
    if (elements.empty()) return new ArrayType(NULL_T);

    UnionType* elementsType = new UnionType({});

    for (Value element : elements)
      elementsType->add(element.typeOf());

    if (elementsType->types.size() == 1)
      return new ArrayType(elementsType->types[0]);
    return new ArrayType(elementsType);
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