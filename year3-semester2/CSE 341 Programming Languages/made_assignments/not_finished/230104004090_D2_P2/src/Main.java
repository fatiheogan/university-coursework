import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.List;

/**
 * ScoreScript — Main Entry Point
 *
 * Pipeline:
 *   lex → parse → type-check → interpret
 *
 * Usage:
 *   java Main <source_file>             // lex, parse, type-check, then run
 *   java Main <source_file> --dump-ast  // lex, parse, print the AST and stop
 *
 * The --dump-ast flag keeps its Part 1 behaviour exactly: the program is
 * parsed and its AST is printed, but it is NOT type-checked or executed.
 * Use it to inspect parser output without running the program.
 *
 * Exit codes:
 *   0 — success (parsed + dumped, or type-checked + executed)
 *   1 — lexer or parser error
 *   2 — usage error (wrong arguments) or file could not be read
 *   3 — type error (static check failed)
 *   4 — runtime error (interpreter aborted)
 *
 * CSE 341 · Spring 2026
 */
public class Main {

    public static void main(String[] args) {

        // ── Argument validation ──
        if (args.length < 1 || args.length > 2) {
            System.err.println("Usage: java Main <source_file> [--dump-ast]");
            System.exit(2);
            return;
        }

        String  sourceFile = args[0];
        boolean dumpAst     = args.length == 2 && args[1].equals("--dump-ast");

        if (args.length == 2 && !dumpAst) {
            System.err.println("Usage: java Main <source_file> [--dump-ast]");
            System.exit(2);
            return;
        }

        // ── Read source ──
        String source;
        try {
            source = new String(Files.readAllBytes(Paths.get(sourceFile)));
        } catch (IOException e) {
            System.err.println("Error: cannot read file '" + sourceFile + "'");
            System.exit(2);
            return;
        }

        // ── 1. Lex ──
        List<Token> tokens;
        try {
            Lexer lexer = new Lexer(source);
            tokens = lexer.tokenize();
        } catch (Lexer.LexerException e) {
            System.err.println(e.getMessage());
            System.exit(1);
            return;
        }

        // ── 2. Parse ──
        AST.Program program;
        try {
            Parser parser = new Parser(tokens);
            program = parser.parse();
        } catch (Parser.ParseException e) {
            System.err.println(e.getMessage());
            System.exit(1);
            return;
        }

        // ── --dump-ast: preserve the exact Part 1 behaviour ──
        // Print the AST and stop. No type-checking, no execution.
        if (dumpAst) {
            System.out.println(program.dump(""));
            return; // exit code 0
        }

        // ── 3. Type-check ──
        // On any type error, print the message and exit before running.
        try {
            new TypeChecker().check(program);
        } catch (TypeChecker.TypeError e) {
            System.err.println(e.getMessage());
            System.exit(3);
            return;
        }

        // ── 4. Interpret ──
        // The program is well-typed; execute it. Runtime errors (division by
        // zero, average/rank of an empty group, other domain errors) are
        // reported here and abort with a distinct exit code.
        try {
            new Interpreter().run(program);
        } catch (Interpreter.RuntimeError e) {
            System.err.println(e.getMessage());
            System.exit(4);
            return;
        }
    }
}
