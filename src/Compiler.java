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

@Command(name = "mango", description = "The mango compiler.", mixinStandardHelpOptions = true, version = "1.0.0")
public class Compiler implements Runnable {

   @Parameters(index = "0..*", description = "The files to compile.")
   private static File[] files = {};

   @Option(names = "--gui", description = "Determins whether mango will show a GUI of the parse tree.", defaultValue = "false")
   private static boolean gui;

   private static File file;

   public static void main(String[] args) throws Exception {

      new CommandLine(new Compiler()).execute(args);

   }

   @Override
   public void run() {

      for (File file : files) {

         if (!file.getName().matches(".+\\.mango")) {

            System.out.println("error: file does not end in '.mango'");
            continue;

         }

         try {

            compile(file);

         }

         catch (IOException e) {

            System.out.println("error: file not found " + file.getPath());

         }

      }

   }

   public static void compile(File file) throws IOException {

      Compiler.file = file;

      MangoLexer lexer = new MangoLexer(CharStreams.fromFileName(file.getAbsolutePath()));
      lexer.removeErrorListeners();
      lexer.addErrorListener(new ErrorListener());

      MangoParser parser = new MangoParser(new CommonTokenStream(lexer));
      parser.removeErrorListeners();
      parser.addErrorListener(new ErrorListener());

      if (gui) {

         ParseTree tree = parser.program();
         new TreeViewer(Arrays.asList(parser.getRuleNames()), tree).open();

      }

   }

   public static String getFileName() {

      return Compiler.file.getName();

   }

}