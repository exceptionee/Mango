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

struct ArrayType : Type {
  Type* elementsType;

  ArrayType(Type* elementsType) : elementsType(elementsType) {}

  bool superset(Type* type) override {
    if (ArrayType* arrayType = dynamic_cast<ArrayType*>(type))
      return elementsType->superset(arrayType->elementsType);

    return false;
  }

  virtual std::string toString() override {
    return elementsType->toString() + "[]";
  }
};

struct : Type {
  bool superset(Type* type) override {
    return true;
  }

  virtual std::string toString() override {
    return "any";
  }
} *ANY_T;

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
Type* INT_T = new Simple("int");
Type* FLOAT_T = new Simple("float");
Type* STRING_T = new Simple("string");
Type* CHAR_T = new Simple("char");
Type* BOOL_T = new Simple("bool");
Type* NULL_T = new Simple("null");