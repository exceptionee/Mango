import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import org.antlr.v4.runtime.CharStreams;
import org.antlr.v4.runtime.CommonTokenStream;
import org.antlr.v4.runtime.IntStream;
import org.antlr.v4.runtime.tree.ParseTree;
import org.antlr.v4.runtime.tree.Tree;
import picocli.CommandLine;
import picocli.CommandLine.Command;
import picocli.CommandLine.Option;
import picocli.CommandLine.Parameters;

@Command(name = "mango", description = "The mango interpreter.", mixinStandardHelpOptions = true, version = "mango v0.1.0a")
public class Runner implements Runnable {
  @Parameters(index = "0..*", description = "The files to interpret.", arity = "0..*")
  private static String[] files = {};
  @Option(names = "--gui", description = "Determines whether mango will show a GUI of the parse tree.", defaultValue = "false")
  private static boolean gui;

  protected static String file = "REPL";

  private static MangoLexer lexer = new MangoLexer(null);
  private static MangoParser parser = new MangoParser(null);
  private static TypeChecker typeChecker = new TypeChecker();
  private static Interpreter interpreter = new Interpreter();

  static {
    lexer.removeErrorListeners();
    lexer.addErrorListener(new ErrorListener());

    parser.removeErrorListeners();
    parser.addErrorListener(new ErrorListener());
  }

  public static void main(String[] args) throws Exception {
    new CommandLine(new Runner()).execute(args);
  }

  @Override
  public void run() {
    if (files.length > 0) {
      for (String file : files) {
        if (!file.endsWith(".mango")) {
          System.out.println("error: file does not end in '.mango'");
          continue;
        }

        try {
          Runner.file = file;
          interpret(CharStreams.fromFileName(file));

          // if (gui)
          //   new TreeViewer(Arrays.asList(MangoParser.ruleNames), tree).open();
        }
        catch (IOException e) {
          System.out.println("error: file not found " + file);
        }
      }
    }
    else {
      try {
        BufferedReader r = new BufferedReader(new InputStreamReader(System.in));

        while (true) {
          System.out.print("> ");
          String input = r.readLine();
    
          if (input == null)
            break;
          else if (input.isEmpty())
            continue;
    
          Object result = interpret(CharStreams.fromString(input));
    
          if (!Error.error)
            System.out.println("\u001b[38;2;225;160;70m" + result + "\u001B[0m");
          else {
            Error.error = false;
            typeChecker.stack.getFirst().entrySet()
              .removeIf(e -> e.getValue().type() == null);
          }
        }
      }
      catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private static Object interpret(IntStream stream) {
    lexer.setInputStream(stream);
    parser.setTokenStream(new CommonTokenStream(lexer));

    Tree tree = parser.program();

    typeChecker.visit((ParseTree) tree);
    return !Error.error? interpreter.visit((ParseTree) tree) : null;
  }
}