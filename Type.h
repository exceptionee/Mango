#include <ostream>
#include <iostream>

struct Type {
  virtual bool superset() = 0;
  virtual bool equals() = 0;

  virtual std::ostream& operator<<(std::ostream& os) = 0;
};

struct ANY {
  bool superset() {
    return true;
  }

  bool equals() {
    return true;
  }

  virtual std::ostream& operator<<(std::ostream& os) {
    return (os << "any");
  }
};

enum Primitive {
  INT, FLOAT, STRING, CHAR, BOOL, _NULL
};