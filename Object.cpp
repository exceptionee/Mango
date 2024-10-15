#include <string>
#include <variant>
#include <memory>
#include <vector>
#include <iostream>
#include "Object.h"

bool Value::operator==(Value v) const {
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

      std::vector<Value> aArray = std::static_pointer_cast<Array>(std::get<std::shared_ptr<Object>>(data))->elements;
      std::vector<Value> bArray = std::static_pointer_cast<Array>(std::get<std::shared_ptr<Object>>(v.data))->elements;
      
      return aArray == bArray;
    }

    default: return true; // when both values are null
  }
}

std::ostream& operator<<(std::ostream& os, const Value v) {
  switch (v.data.index()) {
    case 0: return (os << "\e[38;2;225;160;70m" << std::get<long long>(v.data) << "\e[0m");
    case 1: return (os << "\e[38;2;225;160;70m" << std::get<double>(v.data) << "\e[0m");
    case 3: return (os << "\e[38;2;255;44;44m" << (std::get<bool>(v.data)? "true" : "false") << "\e[0m");
    case 4: {
      if (std::get<std::shared_ptr<Object>>(v.data)->type == OBJECT_STRING)
        return (os << "\e[38;2;34;166;157m" << std::static_pointer_cast<String>(std::get<std::shared_ptr<Object>>(v.data))->chars << "\e[0m");

      os << "[";
      const std::vector<Value> values = std::static_pointer_cast<Array>(std::get<std::shared_ptr<Object>>(v.data))->elements;
      for (int i = 0; i < values.size(); ++i) {
        os << values[i];
        if (i != values.size() - 1) os << ", ";
      }

      return (os << "]");
    }

    default: return (os << "\e[38;2;136;136;136m" << "null" << "\e[0m");
  }
}