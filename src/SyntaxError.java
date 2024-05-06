public class SyntaxError extends Error {
  SyntaxError(String message, Source location) {
    super(message, location);
  }
}