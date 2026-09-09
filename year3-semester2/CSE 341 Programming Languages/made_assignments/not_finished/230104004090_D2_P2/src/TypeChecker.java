import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * ScoreScript Static Type Checker
 *
 * Walks an AST.Program produced by the Parser and rejects programs that are
 * syntactically valid but semantically ill-typed. On the first error it throws
 * a TypeError (a RuntimeException) whose message has the form:
 *
 *     Type error at line X: <description>
 *
 * Type system (name equivalence, no implicit conversions):
 *   - Value types: int, float, string, bool
 *   - void is ONLY a function return type; it is never the type of a value
 *   - Two types are equal iff their names are identical
 *     (so int and float never mix: int + float is an error)
 *
 * Symbol tables:
 *   - functions : a single HashMap<String, FuncDecl>           (top-level only)
 *   - students  : a single HashMap<String, StudentDecl>        (top-level only)
 *   - groups    : a single HashMap<String, GroupDecl>          (top-level only)
 *   - variables : a chain of HashMap-backed Scope objects so that the static
 *                 (lexical) scoping declared in D1 4.6 is honoured. Each
 *                 function body, for-loop body, and if/else branch opens a
 *                 fresh child scope; lookups walk outward to the parent.
 *
 * This file does not modify or depend on changes to any existing source file.
 * It is not wired into Main (that would require editing Main.java); call
 * {@link #check(AST.Program)} from a Part 2 driver when ready.
 *
 * CSE 341 · Spring 2026
 */
public class TypeChecker {

    // ─────────────────────────────────────────────
    // Type name constants
    // ─────────────────────────────────────────────
    private static final String INT    = "int";
    private static final String FLOAT  = "float";
    private static final String STRING = "string";
    private static final String BOOL   = "bool";
    private static final String VOID   = "void";

    /** Sentinel stored in a Scope for a for-loop's student binding.
     *  It is not a real value type, so using it as a value is rejected. */
    private static final String STUDENT_BINDING = "@student";

    // ─────────────────────────────────────────────
    // Global symbol tables
    // ─────────────────────────────────────────────
    private final Map<String, AST.StudentDecl> students  = new HashMap<>();
    private final Map<String, AST.GroupDecl>   groups    = new HashMap<>();
    private final Map<String, AST.FuncDecl>    functions = new HashMap<>();

    // ─────────────────────────────────────────────
    // Variable symbol table — a chain of scopes
    // ─────────────────────────────────────────────
    private static final class Scope {
        final Scope parent;
        final Map<String, String> vars = new HashMap<>(); // name -> type (or @student)
        Scope(Scope parent) { this.parent = parent; }
    }

    private Scope currentScope;

    // Return type of the function whose body is currently being checked
    // (null at the top level — a return there is an error).
    private String currentFunctionReturnType = null;
    private String currentFunctionName       = null;

    // ─────────────────────────────────────────────
    // Error type
    // ─────────────────────────────────────────────
    public static final class TypeError extends RuntimeException {
        public TypeError(String message) { super(message); }
    }

    private TypeError err(int line, String message) {
        return new TypeError("Type error at line " + line + ": " + message);
    }

    // ═════════════════════════════════════════════
    // Public entry point — walk the Program
    // ═════════════════════════════════════════════
    public void check(AST.Program program) {
        currentScope = new Scope(null);   // global scope

        // Pass 1 — collect every top-level declaration's signature so that
        // declarations may reference each other (e.g. mutually recursive
        // functions, a group listed before a student is used in a stmt).
        for (AST.Node d : program.declarations) {
            if (d instanceof AST.StudentDecl) {
                AST.StudentDecl s = (AST.StudentDecl) d;
                if (students.containsKey(s.name) || groups.containsKey(s.name))
                    throw err(s.line, "name '" + s.name + "' is already declared");
                students.put(s.name, s);
            } else if (d instanceof AST.GroupDecl) {
                AST.GroupDecl g = (AST.GroupDecl) d;
                if (groups.containsKey(g.name) || students.containsKey(g.name))
                    throw err(g.line, "name '" + g.name + "' is already declared");
                groups.put(g.name, g);
            } else if (d instanceof AST.FuncDecl) {
                AST.FuncDecl f = (AST.FuncDecl) d;
                if (functions.containsKey(f.name))
                    throw err(f.line, "function '" + f.name + "' is already declared");
                functions.put(f.name, f);
            }
            // VarDecl signatures are registered in Pass 2, in source order,
            // so their initializers can only see earlier global variables
            // (declaration-before-use for globals).
        }

        // Pass 2 — type-check every declaration in source order.
        for (AST.Node d : program.declarations) {
            if (d instanceof AST.GroupDecl) {
                checkGroupDecl((AST.GroupDecl) d);
            } else if (d instanceof AST.VarDecl) {
                checkVarDecl((AST.VarDecl) d);
            } else if (d instanceof AST.FuncDecl) {
                checkFuncDecl((AST.FuncDecl) d);
            }
            // StudentDecl has nothing to type-check: the grammar already
            // guarantees id/firstName/lastName are strings and grades are ints.
        }

        // Then the statement section, in the global scope.
        for (AST.Node s : program.statements) {
            checkStmt(s);
        }
    }

    // ─────────────────────────────────────────────
    // Declarations
    // ─────────────────────────────────────────────

    /** Every member of a group must be a declared student. */
    private void checkGroupDecl(AST.GroupDecl g) {
        for (String member : g.members) {
            if (!students.containsKey(member)) {
                if (groups.containsKey(member))
                    throw err(g.line, "group '" + g.name + "' lists '" + member
                            + "', which is a group, not a student");
                throw err(g.line, "group '" + g.name + "' lists '" + member
                        + "', which is not a declared student");
            }
        }
    }

    /** <value_type> IDENT "=" <expr> — initializer type must equal declared type. */
    private void checkVarDecl(AST.VarDecl v) {
        String initType = inferExpr(v.initializer);
        if (!typeEquals(v.type, initType)) {
            throw err(v.line, "cannot initialize " + v.type + " variable '"
                    + v.name + "' with a value of type " + initType
                    + " (no implicit conversion)");
        }
        declareVar(v.name, v.type, v.line);
    }

    /** Check a function body in a fresh scope with its parameters bound. */
    private void checkFuncDecl(AST.FuncDecl f) {
        Scope saved   = currentScope;
        String savedRt = currentFunctionReturnType;
        String savedFn = currentFunctionName;

        currentScope              = new Scope(saved);          // params + locals
        currentFunctionReturnType = f.returnType;
        currentFunctionName       = f.name;

        for (AST.Param p : f.params) {
            if (currentScope.vars.containsKey(p.name))
                throw err(f.line, "duplicate parameter '" + p.name
                        + "' in function '" + f.name + "'");
            currentScope.vars.put(p.name, p.type);
        }

        for (AST.Node s : f.body) checkStmt(s);

        currentScope              = saved;
        currentFunctionReturnType = savedRt;
        currentFunctionName       = savedFn;
    }

    // ─────────────────────────────────────────────
    // Statements
    // ─────────────────────────────────────────────
    private void checkStmt(AST.Node s) {
        if (s instanceof AST.VarDecl) {
            checkVarDecl((AST.VarDecl) s);

        } else if (s instanceof AST.AssignStmt) {
            checkAssign((AST.AssignStmt) s);

        } else if (s instanceof AST.CurveStmt) {
            AST.CurveStmt c = (AST.CurveStmt) s;
            requireGroup(c.groupName, c.line, "curve");
            String amt = inferExpr(c.amount);
            if (!typeEquals(amt, INT))
                throw err(c.line, "curve amount must be int but is " + amt);

        } else if (s instanceof AST.AverageStmt) {
            AST.AverageStmt a = (AST.AverageStmt) s;
            requireGroup(a.groupName, a.line, "average");

        } else if (s instanceof AST.RankStmt) {
            AST.RankStmt r = (AST.RankStmt) s;
            requireGroup(r.groupName, r.line, "rank");

        } else if (s instanceof AST.TranscriptStmt) {
            AST.TranscriptStmt t = (AST.TranscriptStmt) s;
            requireStudent(t.studentName, t.line);

        } else if (s instanceof AST.IfStmt) {
            checkIf((AST.IfStmt) s);

        } else if (s instanceof AST.ForStmt) {
            checkFor((AST.ForStmt) s);

        } else if (s instanceof AST.PrintStmt) {
            // print accepts any value type; a void result is rejected by inferExpr.
            inferExpr(((AST.PrintStmt) s).value);

        } else if (s instanceof AST.ReturnStmt) {
            checkReturn((AST.ReturnStmt) s);

        } else if (s instanceof AST.CallStmt) {
            // A call used as a statement: its return value is discarded, so a
            // void return is fine here. checkCall validates name/args.
            checkCall(((AST.CallStmt) s).call);

        } else {
            // Should not happen for a well-formed AST.
            throw err(s.line, "unsupported statement: "
                    + s.getClass().getSimpleName());
        }
    }

    private void checkAssign(AST.AssignStmt a) {
        String declared = lookupVar(a.name);
        if (declared == null) {
            // Give the most informative diagnosis possible.
            if (functions.containsKey(a.name))
                throw err(a.line, "cannot assign to function '" + a.name + "'");
            if (groups.containsKey(a.name))
                throw err(a.line, "cannot assign to group '" + a.name + "'");
            if (students.containsKey(a.name))
                throw err(a.line, "cannot assign to student '" + a.name + "'");
            throw err(a.line, "assignment to undeclared variable '" + a.name + "'");
        }
        if (STUDENT_BINDING.equals(declared))
            throw err(a.line, "cannot assign to for-loop student '" + a.name + "'");

        String valueType = inferExpr(a.value);
        if (!typeEquals(declared, valueType))
            throw err(a.line, "cannot assign a value of type " + valueType
                    + " to " + declared + " variable '" + a.name
                    + "' (no implicit conversion)");
    }

    private void checkIf(AST.IfStmt s) {
        for (int i = 0; i < s.conditions.size(); i++) {
            AST.Expr cond = s.conditions.get(i);
            String ct = inferExpr(cond);
            if (!typeEquals(ct, BOOL))
                throw err(cond.line, "if condition must be bool but is " + ct);
            checkBlock(s.branches.get(i));
        }
        if (s.elseBranch != null) checkBlock(s.elseBranch);
    }

    /**
     * ScoreScript's for loop is `for student IDENT in IDENT` — it has no
     * boolean condition, so the "non-bool condition" rule applies only to if /
     * else-if. Here we instead require the iterated name to be a group and we
     * bind the loop variable as a student for the body.
     */
    private void checkFor(AST.ForStmt s) {
        requireGroup(s.groupName, s.line, "for ... in");

        Scope saved = currentScope;
        currentScope = new Scope(saved);
        currentScope.vars.put(s.loopVar, STUDENT_BINDING);
        for (AST.Node b : s.body) checkStmt(b);
        currentScope = saved;
    }

    private void checkReturn(AST.ReturnStmt r) {
        if (currentFunctionReturnType == null)
            throw err(r.line, "return statement outside of a function");

        String got = inferExpr(r.value);
        if (typeEquals(currentFunctionReturnType, VOID))
            throw err(r.line, "void function '" + currentFunctionName
                    + "' must not return a value");
        if (!typeEquals(currentFunctionReturnType, got))
            throw err(r.line, "function '" + currentFunctionName
                    + "' must return " + currentFunctionReturnType
                    + " but returns " + got + " (no implicit conversion)");
    }

    /** Check a brace-delimited block in its own (lexical) child scope. */
    private void checkBlock(List<AST.Node> body) {
        Scope saved = currentScope;
        currentScope = new Scope(saved);
        for (AST.Node s : body) checkStmt(s);
        currentScope = saved;
    }

    // ─────────────────────────────────────────────
    // Expression typing
    // Returns one of int / float / string / bool.
    // Never returns void or @student: using either as a value is an error.
    // ─────────────────────────────────────────────
    private String inferExpr(AST.Expr e) {
        if (e instanceof AST.IntLiteral)    return INT;
        if (e instanceof AST.FloatLiteral)  return FLOAT;
        if (e instanceof AST.StringLiteral) return STRING;
        if (e instanceof AST.BoolLiteral)   return BOOL;

        if (e instanceof AST.IdentExpr)  return inferIdent((AST.IdentExpr) e);
        if (e instanceof AST.CallExpr)   return inferCallValue((AST.CallExpr) e);
        if (e instanceof AST.UnaryExpr)  return inferUnary((AST.UnaryExpr) e);
        if (e instanceof AST.BinaryExpr) return inferBinary((AST.BinaryExpr) e);

        throw err(e.line, "unsupported expression: "
                + e.getClass().getSimpleName());
    }

    private String inferIdent(AST.IdentExpr id) {
        String t = lookupVar(id.name);
        if (t == null) {
            if (functions.containsKey(id.name))
                throw err(id.line, "function '" + id.name
                        + "' must be called with arguments to be used as a value");
            if (groups.containsKey(id.name))
                throw err(id.line, "'" + id.name
                        + "' is a group and cannot be used as a value");
            if (students.containsKey(id.name))
                throw err(id.line, "'" + id.name
                        + "' is a student and cannot be used as a value");
            throw err(id.line, "use of undeclared identifier '" + id.name + "'");
        }
        if (STUDENT_BINDING.equals(t))
            throw err(id.line, "for-loop student '" + id.name
                    + "' cannot be used as a value");
        return t;
    }

    /** A call appearing in expression position — a void return is rejected. */
    private String inferCallValue(AST.CallExpr c) {
        String rt = checkCall(c);
        if (typeEquals(rt, VOID))
            throw err(c.line, "cannot use the result of void function '"
                    + c.name + "' as a value");
        return rt;
    }

    /**
     * Validate a call (callee exists, arity, argument types) and return the
     * function's declared return type (possibly "void").
     */
    private String checkCall(AST.CallExpr c) {
        AST.FuncDecl f = functions.get(c.name);
        if (f == null) {
            if (lookupVar(c.name) != null)
                throw err(c.line, "'" + c.name + "' is a variable, not a function");
            throw err(c.line, "call to undeclared function '" + c.name + "'");
        }
        if (c.args.size() != f.params.size())
            throw err(c.line, "function '" + c.name + "' expects "
                    + f.params.size() + " argument(s) but got " + c.args.size());

        for (int i = 0; i < c.args.size(); i++) {
            String argType   = inferExpr(c.args.get(i));
            String paramType = f.params.get(i).type;
            if (!typeEquals(argType, paramType))
                throw err(c.args.get(i).line, "argument " + (i + 1)
                        + " of '" + c.name + "' expects " + paramType
                        + " but got " + argType + " (no implicit conversion)");
        }
        return f.returnType;
    }

    private String inferUnary(AST.UnaryExpr u) {
        String op = u.op;
        String t  = inferExpr(u.operand);
        if (op.equals("-")) {
            if (!typeEquals(t, INT) && !typeEquals(t, FLOAT))
                throw err(u.line, "unary '-' requires int or float but got " + t);
            return t;
        }
        if (op.equals("!")) {
            if (!typeEquals(t, BOOL))
                throw err(u.line, "unary '!' requires bool but got " + t);
            return BOOL;
        }
        throw err(u.line, "unknown unary operator '" + op + "'");
    }

    private String inferBinary(AST.BinaryExpr b) {
        String op = b.op;
        String lt = inferExpr(b.left);
        String rt = inferExpr(b.right);

        switch (op) {
            // Arithmetic: operands must be the SAME numeric type.
            case "+": case "-": case "*": case "/":
                if (!typeEquals(lt, rt))
                    throw err(b.line, "operator '" + op
                            + "' needs operands of the same type but got "
                            + lt + " and " + rt + " (no implicit conversion)");
                if (!typeEquals(lt, INT) && !typeEquals(lt, FLOAT))
                    throw err(b.line, "operator '" + op
                            + "' requires int or float but got " + lt);
                return lt;

            // Relational: same numeric type, result bool.
            case "<": case "<=": case ">": case ">=":
                if (!typeEquals(lt, rt))
                    throw err(b.line, "operator '" + op
                            + "' needs operands of the same type but got "
                            + lt + " and " + rt + " (no implicit conversion)");
                if (!typeEquals(lt, INT) && !typeEquals(lt, FLOAT))
                    throw err(b.line, "operator '" + op
                            + "' requires int or float but got " + lt);
                return BOOL;

            // Equality: same type (any value type), result bool.
            case "==": case "!=":
                if (!typeEquals(lt, rt))
                    throw err(b.line, "operator '" + op
                            + "' needs operands of the same type but got "
                            + lt + " and " + rt + " (no implicit conversion)");
                return BOOL;

            // Logical: both bool, result bool.
            case "&&": case "||":
                if (!typeEquals(lt, BOOL) || !typeEquals(rt, BOOL))
                    throw err(b.line, "operator '" + op
                            + "' requires bool operands but got "
                            + lt + " and " + rt);
                return BOOL;

            default:
                throw err(b.line, "unknown binary operator '" + op + "'");
        }
    }

    // ─────────────────────────────────────────────
    // Name-resolution helpers
    // ─────────────────────────────────────────────

    private void declareVar(String name, String type, int line) {
        if (currentScope.vars.containsKey(name))
            throw err(line, "variable '" + name
                    + "' is already declared in this scope");
        currentScope.vars.put(name, type);
    }

    /** Walk the scope chain outward (static scoping). */
    private String lookupVar(String name) {
        for (Scope sc = currentScope; sc != null; sc = sc.parent) {
            String t = sc.vars.get(name);
            if (t != null) return t;
        }
        return null;
    }

    private void requireGroup(String name, int line, String where) {
        if (groups.containsKey(name)) return;
        if (students.containsKey(name))
            throw err(line, "'" + name + "' is a student, not a group ("
                    + where + " expects a group)");
        if (functions.containsKey(name))
            throw err(line, "'" + name + "' is a function, not a group ("
                    + where + " expects a group)");
        if (lookupVar(name) != null)
            throw err(line, "'" + name + "' is a variable, not a group ("
                    + where + " expects a group)");
        throw err(line, "use of undeclared group '" + name + "' in "
                + where);
    }

    private void requireStudent(String name, int line) {
        if (students.containsKey(name)) return;
        if (STUDENT_BINDING.equals(lookupVar(name))) return; // for-loop binding
        if (groups.containsKey(name))
            throw err(line, "'" + name
                    + "' is a group, not a student (transcript expects a student)");
        if (functions.containsKey(name))
            throw err(line, "'" + name
                    + "' is a function, not a student (transcript expects a student)");
        if (lookupVar(name) != null)
            throw err(line, "'" + name
                    + "' is a variable, not a student (transcript expects a student)");
        throw err(line, "transcript of undeclared student '" + name + "'");
    }

    /** Name equivalence: types are equal iff their names match exactly. */
    private boolean typeEquals(String a, String b) {
        return a != null && a.equals(b);
    }
}
