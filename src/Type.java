import java.util.Iterator;
import java.util.LinkedHashSet;

public interface Type {
  boolean superset(Type type);

  Type ERROR = null;

  Type ANY = new Type() {
    @Override
    public boolean superset(Type type) {
      return true;
    }

    @Override
    public String toString() {
      return "any";
    }

    @Override
    public boolean equals(Object obj) {
      return true;
    }
  };

  Union NUMBER = new Union(Primitive.INT, Primitive.FLOAT);

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
      return type instanceof Array && this.type.superset(((Array) type).type);
    }

    @Override
    public final boolean equals(Object obj) {
      return obj instanceof Array && type.equals(((Array) obj).type);
    }

    @Override
    public final int hashCode() {
      return type.hashCode();
    }

    @Override
    public String toString() {
      if (type instanceof Union)
        return "(" + type + ")" + "[]";

      return type + "[]";
    }
  }

  class Union extends LinkedHashSet<Type> implements Type {
    public Union(Type... types) {
      for (Type type : types)
        add(type);
    }

    @Override
    public boolean add(Type type) {
      if (type instanceof Union)
        return super.addAll((Union) type);

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
      StringBuilder string = new StringBuilder(iterator.next().toString());
    
      while (iterator.hasNext())
        string.append(" | ").append(iterator.next());

      return string.toString();
    }
  }
}