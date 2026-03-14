#include "Value.h"
#include <gc/gc_allocator.h>
#include <gc.h>
#include <string>
#include <unordered_map>

struct Environment {
  Environment* parent;
  std::unordered_map<
    std::string,
    Value,
    std::hash<std::string>,
    std::equal_to<std::string>,
    gc_allocator<std::pair<std::string, Value>>>
    map;

  void* operator new(size_t size) {
    void* p = GC_malloc(size);
    if (!p) throw std::bad_alloc();
    return p;
  }

  Environment(Environment* parent) : parent(parent) {}

  void set(const std::string& key, Value value) {
    map[key] = value;
  }

  Value& get(const std::string& key) {
    auto result = map.find(key);
    return result != map.end() ? result->second : parent->get(key);
  }
};