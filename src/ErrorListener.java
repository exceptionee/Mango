import org.antlr.v4.runtime.BaseErrorListener;
import org.antlr.v4.runtime.RecognitionException;
import org.antlr.v4.runtime.Recognizer;

public class ErrorListener extends BaseErrorListener {
  @Override
  public void syntaxError(Recognizer<?, ?> recognizer, Object offender,
    int line, int column, String message, RecognitionException e
  ) {
    new SyntaxError(message, new Source(Runner.file, line, ++column));
  }
}