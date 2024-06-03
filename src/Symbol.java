public interface Symbol {
  Type type();
  
  static record Var(Type type) implements Symbol {}
  static record Const(Type type) implements Symbol {}
}