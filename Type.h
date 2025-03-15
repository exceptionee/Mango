#pragma once

#include <ostream>
#include <string>

struct Type {
  virtual bool superset(Type* type) = 0;
  virtual std::string toString() = 0;

  std::ostream& operator<<(std::ostream& os) {
    return (os << toString());
  }
};

struct UnionType : Type {
  std::vector<Type*> types;

  UnionType(std::vector<Type*> types) {
    for (Type* t : types)
      add(t);
  }

  void add(Type* type) {
    if (UnionType* unionType = dynamic_cast<UnionType*>(type)) {
      for (Type* i : unionType->types)
        add(i);
      return;
    }

    if (!superset(type))
      types.push_back(type);
  }

  bool superset(Type* type) override {
    UnionType* unionType = dynamic_cast<UnionType*>(type);

    if (!unionType) {
      for (Type* t : types)
        if (t->superset(type)) return true;
        
      return false;
    }

    for (Type* t : unionType->types)
      if (!superset(t)) return false;

    return true;
  }

  virtual std::string toString() override {
    if (types.empty()) return "";

    std::string result = types[0]->toString();

    for (int i = 1; i < types.size(); ++i)
      result += " | " + types[i]->toString();

    return result;
  }
};

struct ArrayType : Type {
  Type* elementsType;

  ArrayType(Type* elementsType) : elementsType(elementsType) {}

  bool superset(Type* type) override {
    if (ArrayType* arrayType = dynamic_cast<ArrayType*>(type))
      return elementsType->superset(arrayType->elementsType);

    return false;
  }

  virtual std::string toString() override {
    if (UnionType* unionType = dynamic_cast<UnionType*>(elementsType))
      return "(" + elementsType->toString() + ")[]";

    return elementsType->toString() + "[]";
  }
};

struct Simple : Type {
  std::string name;

  Simple(std::string name) : name(name) {}

  bool superset(Type* type) override {
    return this == type;
  }

  virtual std::string toString() override {
    return name;
  }
};

Type* ERROR_T = nullptr;
Type* ANY_T = new Simple("any");
Type* INT_T = new Simple("int");
Type* FLOAT_T = new Simple("float");
Type* STRING_T = new Simple("string");
Type* CHAR_T = new Simple("char");
Type* BOOL_T = new Simple("bool");
Type* NULL_T = new Simple("null");