#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <variant>
#include <vector>

struct Value;

enum ObjectType {
  OBJECT_STRING,
  OBJECT_ARRAY
};

struct Object {
  ObjectType type;

  Object(ObjectType type) : type(type) {}
  virtual ~Object() = default;
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

struct Value {
  std::variant<long long, double, char, bool, std::shared_ptr<Object>, std::monostate> data;

  bool operator==(const Value& v) const {
    if (data.index() != v.data.index()) return false;

    switch (data.index()) {
      case 0: return std::get<long long>(data) == std::get<long long>(v.data);
      case 1: return std::get<double>(data) == std::get<double>(v.data);
      case 3: return std::get<bool>(data) == std::get<bool>(v.data);
      case 4: {
        if (std::get<std::shared_ptr<Object>>(data)->type == OBJECT_STRING) {
          std::string aString = std::static_pointer_cast<String>(std::get<std::shared_ptr<Object>>(data))->chars;
          std::string bString = std::static_pointer_cast<String>(std::get<std::shared_ptr<Object>>(v.data))->chars;
        
          return aString == bString;
        }

        std::shared_ptr<Array> aArray = std::static_pointer_cast<Array>(std::get<std::shared_ptr<Object>>(data));
        std::shared_ptr<Array> bArray = std::static_pointer_cast<Array>(std::get<std::shared_ptr<Object>>(v.data));
        
        return aArray == bArray;
      }

      default: return true; // when both values are null
    }
  }

  friend std::ostream& operator<<(std::ostream& os, const Value& v) {
    switch (v.data.index()) {
      case 0: return (os << "\e[33m" << std::get<long long>(v.data) << "\e[0m");
      case 1: return (os << "\e[33m" << std::get<double>(v.data) << "\e[0m");
      case 3: return (os << "\e[31m" << (std::get<bool>(v.data)? "true" : "false") << "\e[0m");
      case 4: {
        if (std::get<std::shared_ptr<Object>>(v.data)->type == OBJECT_STRING)
          return (os << "\e[32m" << std::static_pointer_cast<String>(std::get<std::shared_ptr<Object>>(v.data))->chars << "\e[0m");

        os << "[";
        const std::vector<Value> values = std::static_pointer_cast<Array>(std::get<std::shared_ptr<Object>>(v.data))->elements;
        for (int i = 0; i < values.size(); ++i) {
          os << values[i];
          if (i != values.size() - 1) os << ", ";
        }

        return (os << "]");
      }

      default: return (os << "\e[38;5;250m" << "null" << "\e[0m");
    }
  }
};