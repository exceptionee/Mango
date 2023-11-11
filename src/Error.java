public abstract class Error {

   private String message;
   private Location location;

   public Error(String message, Location location) {

      this.message = message;
      this.location = location;

   }

   public void print() {

      System.out.println(String.format("%s: %s\n\t%s", this.getClass().getName(), message, location.toString()));

   }
   
}