public class TypeError extends Error {
	public TypeError(String message, Source location) {
		super(message, location);
	}
}