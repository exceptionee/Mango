public class RuntimeError extends Error {
	public RuntimeError(String message, Source location) {
		super(message, location);
    System.exit(0);
	}
}