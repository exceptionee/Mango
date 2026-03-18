#pragma once

#include "ASTNode.h"
#include "Type.h"
#include <gc/gc_allocator.h>
#include <functional>
#include <gc.h>
#include <memory>
#include <string>
#include <variant>
#include <vector>

struct Environment;

struct Object {
  void* operator new(size_t size) {
    void* p = GC_malloc(size);
    if (!p) throw std::bad_alloc();
    return p;
  }
  virtual bool equals(Object& o) = 0;
  virtual Type* typeOf() = 0;
  virtual std::string toString() = 0;
};

struct Value {
  std::variant<long long, double, char, bool, Object*, std::monostate> data;

  bool operator==(const Value& v) const {
    if (data.index() != v.data.index()) return false;

    switch (data.index()) {
      case 0: return std::get<long long>(data) == std::get<long long>(v.data);
      case 1: return std::get<double>(data) == std::get<double>(v.data);
      case 2: return std::get<char>(data) == std::get<char>(v.data);
      case 3: return std::get<bool>(data) == std::get<bool>(v.data);
      case 4:
        return std::get<Object*>(data)->equals(*std::get<Object*>(v.data));
      default: return true; // when both values are null
    }
  }

  Type* typeOf() {
    switch (data.index()) {
      case 0: return INT_T;
      case 1: return FLOAT_T;
      case 2: return CHAR_T;
      case 3: return BOOL_T;
      case 4: return std::get<Object*>(data)->typeOf();
      default: return NULL_T;
    }
  }
};

std::string stringify(const Value& v);

struct Function : Object {
  FunctionDeclaration& declaration;
  Environment* closure;
  std::function<Value(std::vector<Value, gc_allocator<Value>>&)> nativeImpl;

  Function(FunctionDeclaration& declaration, Environment* closure)
    : declaration(declaration), closure(closure) {}

  Function(
    FunctionDeclaration& declaration,
    std::function<Value(std::vector<Value, gc_allocator<Value>>&)> nativeImpl
  )
    : declaration(declaration), nativeImpl(nativeImpl) {}

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
    return "<function: " + declaration.id.lexeme + ">";
  }
};

struct String : Object {
  std::string chars;

  String(std::string const& chars) : chars(chars) {}

  bool equals(Object& o) override {
    if (auto other = dynamic_cast<String*>(&o)) return chars == other->chars;
    return false;
  }

  Type* typeOf() override {
    return STRING_T;
  }

  std::string toString() override {
    return chars;
  }
};

struct Array : Object, std::vector<Value, gc_allocator<Value>> {
  Type* type;

  Array(Type* elementsType) : type(new ArrayType(elementsType)) {}

  bool equals(Object& o) override {
    return this == &o;
  }

  Type* typeOf() override {
    return type;
  }

  std::string toString() override {
    std::string result = "[";

    for (int i = 0; i < this->size(); ++i) {
      result += stringify((*this)[i]);
      if (i != this->size() - 1) result += ", ";
    }

    return (result + "]");
  }
};

std::string stringify(const Value& v) {
  switch (v.data.index()) {
    case 0: return std::to_string(std::get<long long>(v.data));
    case 1: return std::to_string(std::get<double>(v.data));
    case 2: return std::string(1, std::get<char>(v.data));
    case 3: return std::get<bool>(v.data) ? "true" : "false";
    case 4: return std::get<Object*>(v.data)->toString();
    default: return "null";
  }
}

std::string format(const Value& v) {
  switch (v.data.index()) {
    case 0:
    case 1:
    case 2: return "\e[33m" + stringify(v) + "\e[0m";
    case 3: return "\e[31m" + stringify(v) + "\e[0m";
    case 4: {
      Object* obj = std::get<Object*>(v.data);

      if (dynamic_cast<String*>(obj))
        return "\e[32m" + obj->toString() + "\e[0m";
      else if (dynamic_cast<Function*>(obj))
        return "\e[34m" + obj->toString() + "\e[0m";

      auto array = static_cast<Array*>(obj);
      std::string result = "[";

      for (int i = 0; i < array->size(); ++i) {
        result += format((*array)[i]);
        if (i != array->size() - 1) result += ", ";
      }

      return (result + "]");
    }
    default: return "\e[38;5;250mnull\e[0m";
  }
}