import java.util.Arrays;
import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.List;

public interface Type {
  boolean superset(Type type);

  Type ERROR = null;

  Type ANY = new Type() {
    @Override
    public boolean superset(Type type) {
      return true;
    }

    @Override
    public boolean equals(Object obj) {
      return true;
    }
  };

  Union NUMBER = new Union(Arrays.asList(Primitive.INT, Primitive.FLOAT));

  enum Primitive implements Type {
    INT, FLOAT, STRING, CHAR, BOOL, NULL;

    @Override
    public boolean superset(Type type) {
      return this == type;
    }

    @Override
    public String toString() {
      return this.name().toLowerCase();
    }
  }

  record Array(Type type) implements Type {
    @Override
    public boolean superset(Type type) {
      return type.superset(type);
    }

    @Override
    public String toString() {
      if (type instanceof Union)
        return "(" + type + ")" + "[]";

      return type + "[]";
    }
  }

  class Union extends LinkedHashSet<Type> implements Type {
    public Union(List<Type> arrayList) {
      super(arrayList);
    }

    @Override
    public boolean add(Type type) {
      if (type instanceof Union) {
        Iterator<Type> iterator = ((Union) type).iterator();

        while (iterator.hasNext())
          add(iterator.next());
          
        return true;
      }
      return super.add(type);
    }

    @Override
    public boolean superset(Type type) {
      if (type instanceof Union)
        return containsAll((Union) type);

      return contains(type);
    }

    @Override
    public final String toString() {
      Iterator<Type> iterator = this.iterator();
      String string = "";

      for (int i = 0; i < this.size() - 1; i++) {
        Type next = iterator.next();
        string += next + " | ";
      }

      return string + iterator.next();
    }
  }
}