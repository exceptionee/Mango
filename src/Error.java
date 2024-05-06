public abstract class Error {
  static boolean error = false;

  public Error(String message, Source location) {
    error = true;
    System.out.println(String
      .format("%s: %s\n\t%s", this.getClass().getName(), message, location.toString()));
  }
}