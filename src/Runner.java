import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import org.antlr.v4.gui.TreeViewer;
import org.antlr.v4.runtime.CharStreams;
import org.antlr.v4.runtime.CommonTokenStream;
import org.antlr.v4.runtime.tree.ParseTree;
import picocli.CommandLine;
import picocli.CommandLine.Command;
import picocli.CommandLine.Option;
import picocli.CommandLine.Parameters;

@Command(name = "mango", description = "The mango interpreter.", mixinStandardHelpOptions = true, version = "mango v0.1.0a")
public class Runner implements Runnable {
  @Parameters(index = "0..*", description = "The files to interpret.", arity = "1+")
  private static File[] files = {};

  @Option(names = "--gui", description = "Determines whether mango will show a GUI of the parse tree.", defaultValue = "false")
  private static boolean gui;

  private static File file;

  public static void main(String[] args) throws Exception {
    new CommandLine(new Runner()).execute(args);
  }

  @Override
  public void run() {
    for (File file : files) {
      if (!file.getName().endsWith(".mango")) {
        System.out.println("error: file does not end in '.mango'");
        continue;
      }

      try {
        interpret(file);
      }
      catch (IOException e) {
        System.out.println("error: file not found " + file.getPath());
      }
    }
  }

  public static void interpret(File file) throws IOException {
    Runner.file = file;

    // tokenizes the file
    MangoLexer lexer = new MangoLexer(CharStreams.fromFileName(file.getAbsolutePath()));
    lexer.removeErrorListeners();
    lexer.addErrorListener(new ErrorListener());

    // parses the tokens from the lexer
    MangoParser parser = new MangoParser(new CommonTokenStream(lexer));
    parser.removeErrorListeners();
    parser.addErrorListener(new ErrorListener());

    // creates the parse tree
    ParseTree tree = parser.program();

    TypeChecker typeChecker = new TypeChecker();
    // performs type checking
    // long start = System.nanoTime();
    typeChecker.visit(tree);
    // long stop = System.nanoTime();
    // System.out.println(stop - start);

    if (gui) {
      new TreeViewer(Arrays.asList(parser.getRuleNames()), tree).open();
    }

    if (Error.error) return;

    Interpreter interpreter = new Interpreter();
    // // interprets the tree
    try {
      long start = System.nanoTime();
      interpreter.visit(tree);
      long stop = System.nanoTime();
      System.out.println(stop - start);
    } catch (OutOfMemoryError e) {
      new RuntimeError("out of memory", new Source(getFileName(), 0, 0));
    }

    // opens a window with the parse tree if the --gui flag is set
    if (gui) {
      new TreeViewer(Arrays.asList(parser.getRuleNames()), tree).open();
    }
  }

  public static String getFileName() {
    return Runner.file.getName();
  }
}