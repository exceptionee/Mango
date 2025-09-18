#include <unordered_map>
#include <string>

struct Value;

struct Environment {
  std::shared_ptr<Environment> parent;
  std::unordered_map<std::string, Value> map;

  Environment(std::shared_ptr<Environment> parent) : parent(parent) {}

  void set(std::string key, Value value) {
    map[key] = value;
  }

  Value& get(std::string key) {
    auto result = map.find(key);
    return result != map.end()? result->second : parent->get(key);
  }
};