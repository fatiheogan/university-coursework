import java.util.List;
import java.util.Map;

/**
 * ScoreScript AST Node Classes
 * One sealed hierarchy: every grammar rule has a corresponding node.
 * CSE 341 · Spring 2026
 */
public class AST {

    // ─────────────────────────────────────────────
    // Base
    // ─────────────────────────────────────────────

    /** Every node carries the source line it started on for error reporting. */
    public abstract static class Node {
        public final int line;
        protected Node(int line) { this.line = line; }
        public abstract String dump(String indent);
    }

    // ─────────────────────────────────────────────
    // Program
    // ─────────────────────────────────────────────

    /** <program> ::= <decl_section> <stmt_section> */
    public static class Program extends Node {
        public final List<Node> declarations;
        public final List<Node> statements;

        public Program(List<Node> declarations, List<Node> statements, int line) {
            super(line);
            this.declarations = declarations;
            this.statements   = statements;
        }

        @Override
        public String dump(String indent) {
            StringBuilder sb = new StringBuilder();
            sb.append(indent).append("Program\n");
            sb.append(indent).append("  Declarations:\n");
            for (Node d : declarations) sb.append(d.dump(indent + "    ")).append("\n");
            sb.append(indent).append("  Statements:\n");
            for (Node s : statements)   sb.append(s.dump(indent + "    ")).append("\n");
            return sb.toString().stripTrailing();
        }
    }

    // ─────────────────────────────────────────────
    // Declaration Nodes
    // ─────────────────────────────────────────────

    /**
     * <student_decl> ::= "student" IDENT "{"
     *     "id" ":" STRING ","
     *     "firstName" ":" STRING ","
     *     "lastName"  ":" STRING ","
     *     "grades"    ":" "{" <grade_list> "}"
     * "}"
     */
    public static class StudentDecl extends Node {
        public final String              name;
        public final String              id;
        public final String              firstName;
        public final String              lastName;
        public final Map<String,Integer> grades;   // subject -> score

        public StudentDecl(String name, String id, String firstName,
                           String lastName, Map<String,Integer> grades, int line) {
            super(line);
            this.name      = name;
            this.id        = id;
            this.firstName = firstName;
            this.lastName  = lastName;
            this.grades    = grades;
        }

        @Override
        public String dump(String indent) {
            StringBuilder sb = new StringBuilder();
            sb.append(indent).append("StudentDecl name=").append(name)
              .append(" id=").append(id)
              .append(" firstName=").append(firstName)
              .append(" lastName=").append(lastName).append("\n");
            sb.append(indent).append("  grades:");
            grades.forEach((k,v) -> sb.append(" ").append(k).append(":").append(v));
            return sb.toString();
        }
    }

    /**
     * <group_decl> ::= "group" IDENT "=" "[" <ident_list> "]"
     */
    public static class GroupDecl extends Node {
        public final String       name;
        public final List<String> members;

        public GroupDecl(String name, List<String> members, int line) {
            super(line);
            this.name    = name;
            this.members = members;
        }

        @Override
        public String dump(String indent) {
            return indent + "GroupDecl name=" + name + " members=" + members;
        }
    }

    /**
     * <var_decl> ::= <value_type> IDENT "=" <expr>
     */
    public static class VarDecl extends Node {
        public final String   type;
        public final String   name;
        public final Expr     initializer;

        public VarDecl(String type, String name, Expr initializer, int line) {
            super(line);
            this.type        = type;
            this.name        = name;
            this.initializer = initializer;
        }

        @Override
        public String dump(String indent) {
            return indent + "VarDecl type=" + type + " name=" + name + "\n"
                 + initializer.dump(indent + "  ");
        }
    }

    /**
     * <func_decl> ::= "func" IDENT "(" [<param_list>] ")" "->" <type>
     *                     "{" <func_body> "}"
     */
    public static class FuncDecl extends Node {
        public final String       name;
        public final List<Param>  params;
        public final String       returnType;
        public final List<Node>   body;

        public FuncDecl(String name, List<Param> params,
                        String returnType, List<Node> body, int line) {
            super(line);
            this.name       = name;
            this.params     = params;
            this.returnType = returnType;
            this.body       = body;
        }

        @Override
        public String dump(String indent) {
            StringBuilder sb = new StringBuilder();
            sb.append(indent).append("FuncDecl name=").append(name)
              .append(" returnType=").append(returnType).append("\n");
            for (Param p : params)
                sb.append(indent).append("  Param ").append(p.name)
                  .append(":").append(p.type).append("\n");
            for (Node s : body)
                sb.append(s.dump(indent + "  ")).append("\n");
            return sb.toString().stripTrailing();
        }
    }

    /** A single function parameter: name ":" type */
    public static class Param {
        public final String name;
        public final String type;
        public Param(String name, String type) {
            this.name = name;
            this.type = type;
        }
    }

    // ─────────────────────────────────────────────
    // Statement Nodes
    // ─────────────────────────────────────────────

    /** "curve" IDENT "by" <expr> */
    public static class CurveStmt extends Node {
        public final String groupName;
        public final Expr   amount;

        public CurveStmt(String groupName, Expr amount, int line) {
            super(line);
            this.groupName = groupName;
            this.amount    = amount;
        }

        @Override
        public String dump(String indent) {
            return indent + "CurveStmt group=" + groupName + "\n"
                 + amount.dump(indent + "  ");
        }
    }

    /** "average" IDENT */
    public static class AverageStmt extends Node {
        public final String groupName;

        public AverageStmt(String groupName, int line) {
            super(line);
            this.groupName = groupName;
        }

        @Override
        public String dump(String indent) {
            return indent + "AverageStmt group=" + groupName;
        }
    }

    /** "rank" IDENT */
    public static class RankStmt extends Node {
        public final String groupName;

        public RankStmt(String groupName, int line) {
            super(line);
            this.groupName = groupName;
        }

        @Override
        public String dump(String indent) {
            return indent + "RankStmt group=" + groupName;
        }
    }

    /** "transcript" IDENT */
    public static class TranscriptStmt extends Node {
        public final String studentName;

        public TranscriptStmt(String studentName, int line) {
            super(line);
            this.studentName = studentName;
        }

        @Override
        public String dump(String indent) {
            return indent + "TranscriptStmt student=" + studentName;
        }
    }

    /**
     * <if_stmt> ::= "if" <expr> "{" {<stmt>} "}"
     *               { "else if" <expr> "{" {<stmt>} "}" }
     *               [ "else" "{" {<stmt>} "}" ]
     */
    public static class IfStmt extends Node {
        public final List<Expr>       conditions;   // one per if/else-if branch
        public final List<List<Node>> branches;     // body per condition
        public final List<Node>       elseBranch;   // may be null

        public IfStmt(List<Expr> conditions, List<List<Node>> branches,
                      List<Node> elseBranch, int line) {
            super(line);
            this.conditions = conditions;
            this.branches   = branches;
            this.elseBranch = elseBranch;
        }

        @Override
        public String dump(String indent) {
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < conditions.size(); i++) {
                sb.append(indent).append(i == 0 ? "IfStmt" : "ElseIf").append("\n");
                sb.append(conditions.get(i).dump(indent + "  condition: ")).append("\n");
                for (Node s : branches.get(i))
                    sb.append(s.dump(indent + "  ")).append("\n");
            }
            if (elseBranch != null) {
                sb.append(indent).append("Else\n");
                for (Node s : elseBranch)
                    sb.append(s.dump(indent + "  ")).append("\n");
            }
            return sb.toString().stripTrailing();
        }
    }

    /**
     * <for_stmt> ::= "for" "student" IDENT "in" IDENT "{" {<stmt>} "}"
     */
    public static class ForStmt extends Node {
        public final String     loopVar;    // the student binding
        public final String     groupName;
        public final List<Node> body;

        public ForStmt(String loopVar, String groupName, List<Node> body, int line) {
            super(line);
            this.loopVar   = loopVar;
            this.groupName = groupName;
            this.body      = body;
        }

        @Override
        public String dump(String indent) {
            StringBuilder sb = new StringBuilder();
            sb.append(indent).append("ForStmt var=").append(loopVar)
              .append(" in=").append(groupName).append("\n");
            for (Node s : body) sb.append(s.dump(indent + "  ")).append("\n");
            return sb.toString().stripTrailing();
        }
    }

    /** "print" <expr> */
    public static class PrintStmt extends Node {
        public final Expr value;

        public PrintStmt(Expr value, int line) {
            super(line);
            this.value = value;
        }

        @Override
        public String dump(String indent) {
            return indent + "PrintStmt\n" + value.dump(indent + "  ");
        }
    }

    /** "return" <expr> */
    public static class ReturnStmt extends Node {
        public final Expr value;

        public ReturnStmt(Expr value, int line) {
            super(line);
            this.value = value;
        }

        @Override
        public String dump(String indent) {
            return indent + "ReturnStmt\n" + value.dump(indent + "  ");
        }
    }

    /** IDENT "(" [<arg_list>] ")"  — function call used as a statement */
    public static class CallStmt extends Node {
        public final CallExpr call;
        public CallStmt(CallExpr call, int line) {
            super(line);
            this.call = call;
        }
        @Override
        public String dump(String indent) {
            return indent + "CallStmt\n" + call.dump(indent + "  ");
        }
    }

    /** IDENT "=" <expr>  (assignment, not declaration) */
    public static class AssignStmt extends Node {
        public final String name;
        public final Expr   value;

        public AssignStmt(String name, Expr value, int line) {
            super(line);
            this.name  = name;
            this.value = value;
        }

        @Override
        public String dump(String indent) {
            return indent + "AssignStmt name=" + name + "\n"
                 + value.dump(indent + "  ");
        }
    }

    // ─────────────────────────────────────────────
    // Expression Nodes  (<expr> hierarchy)
    // ─────────────────────────────────────────────

    public abstract static class Expr extends Node {
        protected Expr(int line) { super(line); }
    }

    /** Binary expression: left op right */
    public static class BinaryExpr extends Expr {
        public final Expr   left;
        public final String op;
        public final Expr   right;

        public BinaryExpr(Expr left, String op, Expr right, int line) {
            super(line);
            this.left  = left;
            this.op    = op;
            this.right = right;
        }

        @Override
        public String dump(String indent) {
            return indent + "BinaryExpr op=" + op + "\n"
                 + left.dump(indent  + "  left:  ") + "\n"
                 + right.dump(indent + "  right: ");
        }
    }

    /** Unary expression: op operand  (- or !) */
    public static class UnaryExpr extends Expr {
        public final String op;
        public final Expr   operand;

        public UnaryExpr(String op, Expr operand, int line) {
            super(line);
            this.op      = op;
            this.operand = operand;
        }

        @Override
        public String dump(String indent) {
            return indent + "UnaryExpr op=" + op + "\n"
                 + operand.dump(indent + "  ");
        }
    }

    /** Integer literal */
    public static class IntLiteral extends Expr {
        public final int value;

        public IntLiteral(int value, int line) {
            super(line);
            this.value = value;
        }

        @Override
        public String dump(String indent) {
            return indent + "IntLiteral " + value;
        }
    }

    /** Float literal */
    public static class FloatLiteral extends Expr {
        public final double value;

        public FloatLiteral(double value, int line) {
            super(line);
            this.value = value;
        }

        @Override
        public String dump(String indent) {
            return indent + "FloatLiteral " + value;
        }
    }

    /** String literal */
    public static class StringLiteral extends Expr {
        public final String value;

        public StringLiteral(String value, int line) {
            super(line);
            this.value = value;
        }

        @Override
        public String dump(String indent) {
            return indent + "StringLiteral \"" + value + "\"";
        }
    }

    /** Boolean literal: true | false */
    public static class BoolLiteral extends Expr {
        public final boolean value;

        public BoolLiteral(boolean value, int line) {
            super(line);
            this.value = value;
        }

        @Override
        public String dump(String indent) {
            return indent + "BoolLiteral " + value;
        }
    }

    /** Variable reference: IDENT */
    public static class IdentExpr extends Expr {
        public final String name;

        public IdentExpr(String name, int line) {
            super(line);
            this.name = name;
        }

        @Override
        public String dump(String indent) {
            return indent + "IdentExpr " + name;
        }
    }

    /** Function call: IDENT "(" [<arg_list>] ")" */
    public static class CallExpr extends Expr {
        public final String     name;
        public final List<Expr> args;

        public CallExpr(String name, List<Expr> args, int line) {
            super(line);
            this.name = name;
            this.args = args;
        }

        @Override
        public String dump(String indent) {
            StringBuilder sb = new StringBuilder();
            sb.append(indent).append("CallExpr name=").append(name).append("\n");
            for (Expr a : args) sb.append(a.dump(indent + "  ")).append("\n");
            return sb.toString().stripTrailing();
        }
    }
}
