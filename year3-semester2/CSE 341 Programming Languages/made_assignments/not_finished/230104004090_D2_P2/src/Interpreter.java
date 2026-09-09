import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;

/**
 * ScoreScript Tree-Walking Interpreter
 *
 * Executes an AST.Program that has already been parsed and type-checked.
 * The interpreter assumes the program is well-typed (run {@link TypeChecker}
 * first); it still raises runtime errors for conditions a static checker
 * cannot catch — division by zero, average/rank of an empty group, and
 * other domain errors.
 *
 * Runtime errors are RuntimeExceptions of the form:
 *     Runtime error at line X: <description>
 *
 * Execution model
 * ----------------
 *   - students : Map<String, StudentState>  — a *mutable copy* of each
 *                StudentDecl's grades, so `curve` does not touch the AST.
 *   - groups   : Map<String, List<String>>  — group name -> member names.
 *   - functions: Map<String, FuncDecl>      — stored separately; each call
 *                runs with its OWN local scope whose parent is the GLOBAL
 *                scope (static scoping: a function sees globals, never
 *                another function's locals or its caller's locals).
 *   - variables: a chain of Scope objects (a scope stack), identical in
 *                spirit to the one in TypeChecker.
 *
 * Numeric values are Java Integer (ScoreScript int) or Double (float).
 * Because the type checker forbids mixed-mode arithmetic, the two never
 * meet in one operator; int / int is integer division.
 *
 * This file does not modify or depend on changes to any existing source
 * file. Call {@link #run(AST.Program)} from a Part 2 driver.
 *
 * CSE 341 · Spring 2026
 */
public class Interpreter {

    // ─────────────────────────────────────────────
    // Runtime student state (a mutable grade copy)
    // ─────────────────────────────────────────────
    private static final class StudentState {
        final String id;
        final String firstName;
        final String lastName;
        /** subject -> score, kept in declared order for stable transcripts. */
        final LinkedHashMap<String, Integer> grades;

        StudentState(AST.StudentDecl d) {
            this.id        = d.id;
            this.firstName = d.firstName;
            this.lastName  = d.lastName;
            this.grades    = new LinkedHashMap<>(d.grades);
        }

        String fullName() { return firstName + " " + lastName; }
    }

    // ─────────────────────────────────────────────
    // Symbol tables
    // ─────────────────────────────────────────────
    private final Map<String, StudentState>  students  = new HashMap<>();
    private final Map<String, List<String>>  groups    = new HashMap<>();
    private final Map<String, AST.FuncDecl>  functions = new HashMap<>();

    // Scope chain (the scope stack).
    private static final class Scope {
        final Scope parent;
        final Map<String, Object> vars = new HashMap<>(); // name -> value
        Scope(Scope parent) { this.parent = parent; }
    }

    /** The global scope — every function's local scope chains to THIS one,
     *  not to the caller's scope, which is what makes scoping static. */
    private Scope globalScope;
    private Scope currentScope;

    // ─────────────────────────────────────────────
    // Errors and control-flow signals
    // ─────────────────────────────────────────────
    public static final class RuntimeError extends RuntimeException {
        public RuntimeError(String message) { super(message); }
    }

    private RuntimeError err(int line, String message) {
        return new RuntimeError("Runtime error at line " + line + ": " + message);
    }

    /** Thrown by `return` to unwind out of the executing function body. */
    private static final class ReturnSignal extends RuntimeException {
        final Object value;             // null for a void return
        ReturnSignal(Object value) {
            super(null, null, false, false);
            this.value = value;
        }
    }

    // ═════════════════════════════════════════════
    // Public entry point
    // ═════════════════════════════════════════════
    public void run(AST.Program program) {
        globalScope  = new Scope(null);
        currentScope = globalScope;

        // Pass 1: register students, groups, functions.
        for (AST.Node d : program.declarations) {
            if (d instanceof AST.StudentDecl) {
                AST.StudentDecl s = (AST.StudentDecl) d;
                students.put(s.name, new StudentState(s));
            } else if (d instanceof AST.GroupDecl) {
                AST.GroupDecl g = (AST.GroupDecl) d;
                groups.put(g.name, new ArrayList<>(g.members));
            } else if (d instanceof AST.FuncDecl) {
                AST.FuncDecl f = (AST.FuncDecl) d;
                functions.put(f.name, f);
            }
        }

        // Pass 2: execute variable declarations (initializers) in order.
        for (AST.Node d : program.declarations) {
            if (d instanceof AST.VarDecl) {
                execVarDecl((AST.VarDecl) d);
            }
        }

        // Pass 3: execute the statement section.
        for (AST.Node s : program.statements) {
            execStmt(s);
        }
    }

    // ─────────────────────────────────────────────
    // Statements
    // ─────────────────────────────────────────────
    private void execStmt(AST.Node s) {
        if (s instanceof AST.VarDecl) {
            execVarDecl((AST.VarDecl) s);

        } else if (s instanceof AST.AssignStmt) {
            execAssign((AST.AssignStmt) s);

        } else if (s instanceof AST.CurveStmt) {
            execCurve((AST.CurveStmt) s);

        } else if (s instanceof AST.AverageStmt) {
            execAverage((AST.AverageStmt) s);

        } else if (s instanceof AST.RankStmt) {
            execRank((AST.RankStmt) s);

        } else if (s instanceof AST.TranscriptStmt) {
            execTranscript((AST.TranscriptStmt) s);

        } else if (s instanceof AST.IfStmt) {
            execIf((AST.IfStmt) s);

        } else if (s instanceof AST.ForStmt) {
            execFor((AST.ForStmt) s);

        } else if (s instanceof AST.PrintStmt) {
            Object v = eval(((AST.PrintStmt) s).value);
            System.out.println(stringify(v));

        } else if (s instanceof AST.ReturnStmt) {
            AST.ReturnStmt r = (AST.ReturnStmt) s;
            throw new ReturnSignal(eval(r.value));

        } else if (s instanceof AST.CallStmt) {
            callFunction(((AST.CallStmt) s).call);     // discard return value

        } else {
            throw err(s.line, "unsupported statement: "
                    + s.getClass().getSimpleName());
        }
    }

    private void execVarDecl(AST.VarDecl v) {
        Object val = eval(v.initializer);
        currentScope.vars.put(v.name, val);
    }

    private void execAssign(AST.AssignStmt a) {
        Object val = eval(a.value);
        // Assign into the nearest enclosing scope that declares the name.
        for (Scope sc = currentScope; sc != null; sc = sc.parent) {
            if (sc.vars.containsKey(a.name)) {
                sc.vars.put(a.name, val);
                return;
            }
        }
        // Should not happen after type-checking, but stay defensive.
        throw err(a.line, "assignment to undeclared variable '" + a.name + "'");
    }

    // ── Domain operations ──

    private void execCurve(AST.CurveStmt c) {
        List<String> members = requireGroup(c.groupName, c.line);
        Object amtObj = eval(c.amount);
        int amount = ((Number) amtObj).intValue();
        for (String name : members) {
            StudentState st = students.get(name);
            if (st == null)
                throw err(c.line, "group '" + c.groupName
                        + "' refers to unknown student '" + name + "'");
            for (Map.Entry<String, Integer> g : st.grades.entrySet()) {
                int curved = g.getValue() + amount;
                if (curved < 0)   curved = 0;
                if (curved > 100) curved = 100;
                g.setValue(curved);
            }
        }
    }

    private void execAverage(AST.AverageStmt a) {
        List<String> members = requireGroup(a.groupName, a.line);
        if (members.isEmpty())
            throw err(a.line, "cannot take the average of empty group '"
                    + a.groupName + "'");
        double total = 0.0;
        for (String name : members) {
            StudentState st = students.get(name);
            if (st == null)
                throw err(a.line, "group '" + a.groupName
                        + "' refers to unknown student '" + name + "'");
            total += studentAverage(st, a.line);
        }
        double groupAvg = total / members.size();
        System.out.printf(Locale.ROOT, "Average of %s: %.2f%n", a.groupName, groupAvg);
    }

    private void execRank(AST.RankStmt r) {
        List<String> members = requireGroup(r.groupName, r.line);
        if (members.isEmpty())
            throw err(r.line, "cannot rank empty group '" + r.groupName + "'");

        List<String> ordered = new ArrayList<>(members);
        // Sort by average descending; stable order keeps declaration order
        // for ties.
        ordered.sort((n1, n2) -> {
            double a1 = studentAverage(students.get(n1), r.line);
            double a2 = studentAverage(students.get(n2), r.line);
            return Double.compare(a2, a1);
        });

        System.out.println("Ranking of " + r.groupName + ":");
        int pos = 1;
        for (String name : ordered) {
            StudentState st = students.get(name);
            System.out.printf(Locale.ROOT, "  %d. %s (%.2f)%n",
                    pos++, st.fullName(), studentAverage(st, r.line));
        }
    }

    private void execTranscript(AST.TranscriptStmt t) {
        StudentState st = resolveStudentName(t.studentName, t.line);
        System.out.println("Transcript for " + st.id + " — " + st.fullName());
        for (Map.Entry<String, Integer> g : st.grades.entrySet()) {
            System.out.println("  " + g.getKey() + ": " + g.getValue());
        }
        System.out.printf(Locale.ROOT, "  Average: %.2f%n", studentAverage(st, t.line));
    }

    /** Mean of a student's grade values; guards the empty-grades case. */
    private double studentAverage(StudentState st, int line) {
        int n = st.grades.size();
        if (n == 0)
            throw err(line, "student '" + st.id
                    + "' has no grades to average (division by zero)");
        long sum = 0;
        for (int v : st.grades.values()) sum += v;
        return (double) sum / n;
    }

    // ── Control flow ──

    private void execIf(AST.IfStmt s) {
        for (int i = 0; i < s.conditions.size(); i++) {
            Object cond = eval(s.conditions.get(i));
            if (asBool(cond, s.conditions.get(i).line)) {
                execBlock(s.branches.get(i));
                return;
            }
        }
        if (s.elseBranch != null) execBlock(s.elseBranch);
    }

    private void execFor(AST.ForStmt s) {
        List<String> members = requireGroup(s.groupName, s.line);
        for (String studentName : members) {
            StudentState st = students.get(studentName);
            if (st == null)
                throw err(s.line, "group '" + s.groupName
                        + "' refers to unknown student '" + studentName + "'");
            Scope saved = currentScope;
            currentScope = new Scope(saved);
            currentScope.vars.put(s.loopVar, studentName); // bind by name
            try {
                for (AST.Node b : s.body) execStmt(b);
            } finally {
                currentScope = saved;
            }
        }
    }

    /** Run a brace block in a fresh child scope (block-level lexical scope). */
    private void execBlock(List<AST.Node> body) {
        Scope saved = currentScope;
        currentScope = new Scope(saved);
        try {
            for (AST.Node s : body) execStmt(s);
        } finally {
            currentScope = saved;
        }
    }

    // ─────────────────────────────────────────────
    // Expression evaluation
    // ─────────────────────────────────────────────
    private Object eval(AST.Expr e) {
        if (e instanceof AST.IntLiteral)
            return ((AST.IntLiteral) e).value;        // autobox to Integer
        if (e instanceof AST.FloatLiteral)
            return ((AST.FloatLiteral) e).value;      // autobox to Double
        if (e instanceof AST.StringLiteral)
            return ((AST.StringLiteral) e).value;
        if (e instanceof AST.BoolLiteral)
            return ((AST.BoolLiteral) e).value;

        if (e instanceof AST.IdentExpr)   return evalIdent((AST.IdentExpr) e);
        if (e instanceof AST.CallExpr)    return evalCall((AST.CallExpr) e);
        if (e instanceof AST.UnaryExpr)   return evalUnary((AST.UnaryExpr) e);
        if (e instanceof AST.BinaryExpr)  return evalBinary((AST.BinaryExpr) e);

        throw err(e.line, "unsupported expression: "
                + e.getClass().getSimpleName());
    }

    private Object evalIdent(AST.IdentExpr id) {
        for (Scope sc = currentScope; sc != null; sc = sc.parent) {
            if (sc.vars.containsKey(id.name)) return sc.vars.get(id.name);
        }
        throw err(id.line, "use of undeclared identifier '" + id.name + "'");
    }

    private Object evalCall(AST.CallExpr c) {
        Object result = callFunction(c);
        if (result == null)
            throw err(c.line, "void function '" + c.name
                    + "' produced no value to use in an expression");
        return result;
    }

    /**
     * Invoke a function. A fresh local scope is created whose parent is the
     * GLOBAL scope (static scoping) — not currentScope. Returns the function's
     * value, or null for a void function that fell off the end / returned
     * nothing.
     */
    private Object callFunction(AST.CallExpr c) {
        AST.FuncDecl f = functions.get(c.name);
        if (f == null)
            throw err(c.line, "call to undeclared function '" + c.name + "'");
        if (c.args.size() != f.params.size())
            throw err(c.line, "function '" + c.name + "' expects "
                    + f.params.size() + " argument(s) but got " + c.args.size());

        // Evaluate arguments in the caller's scope first.
        List<Object> argVals = new ArrayList<>();
        for (AST.Expr a : c.args) argVals.add(eval(a));

        Scope saved = currentScope;
        Scope callScope = new Scope(globalScope);   // ← static scoping
        for (int i = 0; i < f.params.size(); i++) {
            callScope.vars.put(f.params.get(i).name, argVals.get(i));
        }

        currentScope = callScope;
        try {
            for (AST.Node s : f.body) execStmt(s);
            return null;                            // fell off the end (void)
        } catch (ReturnSignal ret) {
            return ret.value;
        } finally {
            currentScope = saved;
        }
    }

    private Object evalUnary(AST.UnaryExpr u) {
        Object v = eval(u.operand);
        switch (u.op) {
            case "-":
                if (v instanceof Integer) return -((Integer) v);
                if (v instanceof Double)  return -((Double) v);
                throw err(u.line, "unary '-' applied to non-numeric value");
            case "!":
                return !asBool(v, u.line);
            default:
                throw err(u.line, "unknown unary operator '" + u.op + "'");
        }
    }

    private Object evalBinary(AST.BinaryExpr b) {
        // Short-circuit logical operators.
        if (b.op.equals("&&")) {
            if (!asBool(eval(b.left), b.line)) return false;
            return asBool(eval(b.right), b.line);
        }
        if (b.op.equals("||")) {
            if (asBool(eval(b.left), b.line)) return true;
            return asBool(eval(b.right), b.line);
        }

        Object l = eval(b.left);
        Object r = eval(b.right);

        switch (b.op) {
            case "+": case "-": case "*": case "/":
                return arithmetic(b.op, l, r, b.line);

            case "<": case "<=": case ">": case ">=": {
                double dl = ((Number) l).doubleValue();
                double dr = ((Number) r).doubleValue();
                switch (b.op) {
                    case "<":  return dl <  dr;
                    case "<=": return dl <= dr;
                    case ">":  return dl >  dr;
                    default:   return dl >= dr;
                }
            }

            case "==": return objEquals(l, r);
            case "!=": return !objEquals(l, r);

            default:
                throw err(b.line, "unknown binary operator '" + b.op + "'");
        }
    }

    private Object arithmetic(String op, Object l, Object r, int line) {
        if (l instanceof Integer && r instanceof Integer) {
            int a = (Integer) l, c = (Integer) r;
            switch (op) {
                case "+": return a + c;
                case "-": return a - c;
                case "*": return a * c;
                case "/":
                    if (c == 0)
                        throw err(line, "division by zero");
                    return a / c;                  // integer division
            }
        }
        if (l instanceof Double && r instanceof Double) {
            double a = (Double) l, c = (Double) r;
            switch (op) {
                case "+": return a + c;
                case "-": return a - c;
                case "*": return a * c;
                case "/":
                    if (c == 0.0)
                        throw err(line, "division by zero");
                    return a / c;
            }
        }
        throw err(line, "operator '" + op
                + "' applied to incompatible operand types");
    }

    private boolean objEquals(Object l, Object r) {
        if (l == null || r == null) return l == r;
        return l.equals(r);
    }

    // ─────────────────────────────────────────────
    // Helpers
    // ─────────────────────────────────────────────

    private boolean asBool(Object v, int line) {
        if (v instanceof Boolean) return (Boolean) v;
        throw err(line, "expected a bool value but got "
                + describeValue(v));
    }

    private List<String> requireGroup(String name, int line) {
        List<String> g = groups.get(name);
        if (g == null)
            throw err(line, "use of undeclared group '" + name + "'");
        return g;
    }

    /** Resolve a transcript target: a direct student name OR a for-loop
     *  variable whose value is a student name. */
    private StudentState resolveStudentName(String name, int line) {
        StudentState direct = students.get(name);
        if (direct != null) return direct;

        for (Scope sc = currentScope; sc != null; sc = sc.parent) {
            if (sc.vars.containsKey(name)) {
                Object v = sc.vars.get(name);
                if (v instanceof String && students.containsKey(v))
                    return students.get(v);
                throw err(line, "'" + name
                        + "' is not bound to a student");
            }
        }
        throw err(line, "transcript of unknown student '" + name + "'");
    }

    private String describeValue(Object v) {
        if (v instanceof Integer) return "int";
        if (v instanceof Double)  return "float";
        if (v instanceof String)  return "string";
        if (v instanceof Boolean) return "bool";
        return "unknown";
    }

    /** Printed form for `print`. Floats show one decimal minimum. */
    private String stringify(Object v) {
        if (v == null)            return "void";
        if (v instanceof Double) {
            double d = (Double) v;
            if (d == Math.floor(d) && !Double.isInfinite(d))
                return String.valueOf((long) d) + ".0";
            return String.valueOf(d);
        }
        return String.valueOf(v);
    }
}
