#include <unordered_map>
#include <string>
#include <ostream>

struct Value;

struct Environment {
  Environment* parent;
  std::unordered_map<std::string, Value> map;

  Environment(Environment* parent) : parent(parent) {}

  void set(std::string key, Value value) {
    map[key] = value;
  }

  Value& get(std::string key) {
    auto result = map.find(key);
    return result != map.end()? result->second : parent->get(key);
  }
};