#pragma once

#include <ostream>
#include <string>

struct Type {
  virtual bool equals(Type* type) = 0;
  virtual bool superset(Type* type) = 0;
  virtual std::string toString() = 0;

  std::ostream& operator<<(std::ostream& os) {
    return (os << toString());
  }
};

struct : Type {
  bool equals(Type* type) override {
    return this == type;
  }

  bool superset(Type* type) override {
    return true;
  }

  virtual std::string toString() override {
    return "any";
  }
} Any;

struct FunctionType : Type {
  std::vector<Type*> args;
  Type* returnType;

  FunctionType(std::vector<Type*> args, Type* returnType) : args(args), returnType(returnType) {}

  bool equals(Type* type) override {
    if (FunctionType* functionType = dynamic_cast<FunctionType*>(type)) {
      if (args.size() != functionType->args.size()) return false;

      for (int i = 0; i < args.size(); ++i)
        if (!(args[i]->equals(functionType->args[i]))) return false;

      return returnType->equals(functionType->returnType);
    }
  
    return false;
  }

  bool superset(Type* type) override {
    return equals(type);
  }
  
  virtual std::string toString() override {
    std::string result = "(";

    for (auto arg : args)
      result += arg->toString() + ", ";

    if (args.size() > 0) result.erase(result.size() - 2);
    return result += ") -> " + returnType->toString();
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

  bool equals(Type* type) override {
    if (UnionType* unionType = dynamic_cast<UnionType*>(type))
      return superset(unionType) && unionType->superset(this);

    return false;
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

  bool equals(Type* type) override {
    if (ArrayType* arrayType = dynamic_cast<ArrayType*>(type))
      return elementsType->equals(arrayType->elementsType);

    return false;
  }

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

struct Primitive : Type {
  std::string name;

  Primitive(std::string name) : name(name) {}

  bool equals(Type* type) override {
    return this == type;
  }

  bool superset(Type* type) override {
    return this == type;
  }

  virtual std::string toString() override {
    return name;
  }
};

Type* ERROR_T = nullptr;
Type* ANY_T = &Any;
Type* INT_T = new Primitive("int");
Type* FLOAT_T = new Primitive("float");
Type* STRING_T = new Primitive("string");
Type* CHAR_T = new Primitive("char");
Type* BOOL_T = new Primitive("bool");
Type* NULL_T = new Primitive("null");