#pragma once

#include <string>
#include <variant>
#include <memory>
#include <vector>

enum ObjectType {
  OBJECT_STRING,
  OBJECT_ARRAY
};

struct Object {
  ObjectType type;

  Object(ObjectType type) : type(type) {}
  virtual ~Object() = default;
};

struct Value {
  std::variant<long long, double, char, bool, std::shared_ptr<Object>, std::monostate> data;

  bool operator==(Value v) const;
  friend std::ostream& operator<<(std::ostream& os, const Value v);
};

struct String : Object {
  std::string chars;

  String(std::string const& chars) : Object {OBJECT_STRING}, chars(chars) {}
  String(std::string_view const& chars) : Object {OBJECT_STRING}, chars(chars) {}
};

struct Array : Object {
  std::vector<Value> elements;

  Array(std::vector<Value> const& elements)
    : Object{ObjectType::OBJECT_ARRAY}, elements(elements) {}
};