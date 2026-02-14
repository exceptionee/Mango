#include <fstream>
#include <iostream>
#include <string>
#include <chrono>
#include "Value.h"
#include "Interpreter.h"

std::string declarations = R"(
  func print(value: any): null {}
  func println(value: any): null {}
  func input(): string {}
  func clock(): int {}
  func sizeof(value: any): int {}
  func read(file: string): char[] {}
  func write(file: string, content: char[]): null {}
  func typeof(value: any): string {}
  func splice(array: any, start: int, deleteCount: int, insert: any): any {}
  func push(array: any, value: any): any {
    return splice(array, sizeof(array), 0, [value]);
  }
  func pop(array: any): any {
    return splice(array, sizeof(array) - 1, 1, []);
  }
)";

void registerBuiltins() {
  builtins["print"] = [](std::vector<Value>& args) {
    std::cout << args[0].toString();
    return Value{std::monostate{}};
  };
  
  builtins["println"] = [](std::vector<Value>& args) {
    std::cout << args[0].toString() << std::endl;
    return Value{std::monostate{}};
  };

  builtins["input"] = [](std::vector<Value>& args) {
    std::string input;
    std::getline(std::cin, input);
    return Value{std::make_shared<String>(input)};
  };

  builtins["clock"] = [](std::vector<Value>& args) {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return Value{ms};
  };

  builtins["sizeof"] = [](std::vector<Value>& args) {
    if (args[0].data.index() == 4) {
      std::shared_ptr<Object> object = std::get<std::shared_ptr<Object>>(args[0].data);
      
      if (String* string = dynamic_cast<String*>(object.get()))
        return Value{(long long) string->chars.size()};
      else if (Array* array = dynamic_cast<Array*>(object.get()))
        return Value{(long long) array->elements.size()};
    }
    return Value{static_cast<long long>(sizeof(args[0].data))};
  };
  
  builtins["read"] = [](std::vector<Value>& args) {
    std::string filename =
      std::static_pointer_cast<String>(std::get<std::shared_ptr<Object>>(args[0].data))->chars;
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (!file.is_open())
      throw "failed to open file '" + filename + "'";

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<Value> content;
    content.reserve(size);

    char c;
    while (file.get(c)) {
      content.push_back(Value{c});
    }

    file.close();

    return Value{std::make_shared<Array>(content)};
  };

  builtins["write"] = [](std::vector<Value>& args) {
    std::string filename =
      std::static_pointer_cast<String>(std::get<std::shared_ptr<Object>>(args[0].data))->chars;
    std::vector<Value>& content =
      std::static_pointer_cast<Array>(std::get<std::shared_ptr<Object>>(args[1].data))->elements;

    std::ofstream file(filename, std::ios::binary);

    if (!file.is_open())
      throw "failed to open file '" + filename + "'";

    for (Value& c : content)
      file.put(std::get<char>(c.data));

    file.close();

    return Value{std::monostate{}};
  };

  builtins["typeof"] = [](std::vector<Value>& args) {
    return Value{std::make_shared<String>(args[0].typeOf()->toString())};
  };

  builtins["splice"] = [](std::vector<Value>& args) {
    if (args[0].data.index() != 4 || args[3].data.index() != 4)
      throw "first and fourth arguments to splice must be arrays";

    Array* array =
      dynamic_cast<Array*>(std::get<std::shared_ptr<Object>>(args[0].data).get());
    Array* insert =
      dynamic_cast<Array*>(std::get<std::shared_ptr<Object>>(args[3].data).get());

    if (!array || !insert)
      throw "first and fourth arguments to splice must be arrays";

    int start = std::get<long long>(args[1].data);
    int deleteCount = std::get<long long>(args[2].data);

    start = std::clamp(start, 0, (int) array->elements.size());
    deleteCount = std::clamp(deleteCount, 0, (int) array->elements.size() - start);

    array->elements.erase(array->elements.begin() + start, array->elements.begin() + start + deleteCount);
    array->elements.insert(array->elements.begin() + start, insert->elements.begin(), insert->elements.end());

    return args[0];
  };
}