import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * ScoreScript Parser
 * Recursive-descent parser following the final EBNF grammar.
 * Produces an AST.Program on success.
 * Rejects invalid programs with a ParseException carrying a line number.
 * CSE 341 · Spring 2026
 */
public class Parser {

    // ─────────────────────────────────────────────
    // State
    // ─────────────────────────────────────────────
    private final List<Token> tokens;
    private int               pos;

    public Parser(List<Token> tokens) {
        this.tokens = tokens;
        this.pos    = 0;
    }

    // ─────────────────────────────────────────────
    // Public Entry Point
    // ─────────────────────────────────────────────
    public AST.Program parse() {
        int line = peek().line;
        List<AST.Node> decls = parseDeclSection();
        List<AST.Node> stmts = parseStmtSection();
        expect(TokenType.EOF);
        return new AST.Program(decls, stmts, line);
    }

    // ─────────────────────────────────────────────
    // <decl_section> ::= { <student_decl> | <group_decl> | <var_decl> | <func_decl> }
    // ─────────────────────────────────────────────
    private List<AST.Node> parseDeclSection() {
        List<AST.Node> decls = new ArrayList<>();

        while (isDeclStart()) {
            switch (peek().type) {
                case STUDENT:  decls.add(parseStudentDecl()); break;
                case GROUP:    decls.add(parseGroupDecl());   break;
                case FUNC:     decls.add(parseFuncDecl());    break;
                default:       decls.add(parseVarDecl());     break; // INT_TYPE / FLOAT_TYPE / ...
            }
        }

        return decls;
    }

    private boolean isDeclStart() {
        switch (peek().type) {
            case STUDENT:
            case GROUP:
            case FUNC:
            case INT_TYPE:
            case FLOAT_TYPE:
            case STRING_TYPE:
            case BOOL_TYPE:
                return true;
            default:
                return false;
        }
    }

    // ─────────────────────────────────────────────
    // <student_decl>
    // ─────────────────────────────────────────────
    private AST.StudentDecl parseStudentDecl() {
        int line = peek().line;
        expect(TokenType.STUDENT);
        String name = expect(TokenType.IDENT).value;
        expect(TokenType.LBRACE);

        // Fixed mandatory field order: id, firstName, lastName, grades
        expect(TokenType.KW_ID);        expect(TokenType.COLON);
        String id = expect(TokenType.STRING).value;
        expect(TokenType.COMMA);

        expect(TokenType.KW_FIRST_NAME); expect(TokenType.COLON);
        String firstName = expect(TokenType.STRING).value;
        expect(TokenType.COMMA);

        expect(TokenType.KW_LAST_NAME);  expect(TokenType.COLON);
        String lastName = expect(TokenType.STRING).value;
        expect(TokenType.COMMA);

        expect(TokenType.KW_GRADES);    expect(TokenType.COLON);
        Map<String,Integer> grades = parseGradeBlock();

        expect(TokenType.RBRACE);
        return new AST.StudentDecl(name, id, firstName, lastName, grades, line);
    }

    /** "{" <grade_list> "}"   grade_list = IDENT ":" INT { "," IDENT ":" INT } */
    private Map<String,Integer> parseGradeBlock() {
        expect(TokenType.LBRACE);
        Map<String,Integer> grades = new LinkedHashMap<>();

        String subject = expect(TokenType.IDENT).value;
        expect(TokenType.COLON);
        int score = Integer.parseInt(expect(TokenType.INT).value);
        grades.put(subject, score);

        while (check(TokenType.COMMA)) {
            advance();
            subject = expect(TokenType.IDENT).value;
            expect(TokenType.COLON);
            score = Integer.parseInt(expect(TokenType.INT).value);
            grades.put(subject, score);
        }

        expect(TokenType.RBRACE);
        return grades;
    }

    // ─────────────────────────────────────────────
    // <group_decl> ::= "group" IDENT "=" "[" <ident_list> "]"
    // ─────────────────────────────────────────────
    private AST.GroupDecl parseGroupDecl() {
        int line = peek().line;
        expect(TokenType.GROUP);
        String name = expect(TokenType.IDENT).value;
        expect(TokenType.EQ);
        expect(TokenType.LBRACKET);

        List<String> members = new ArrayList<>();
        members.add(expect(TokenType.IDENT).value);
        while (check(TokenType.COMMA)) {
            advance();
            members.add(expect(TokenType.IDENT).value);
        }

        expect(TokenType.RBRACKET);
        return new AST.GroupDecl(name, members, line);
    }

    // ─────────────────────────────────────────────
    // <var_decl> ::= <value_type> IDENT "=" <expr>
    // ─────────────────────────────────────────────
    private AST.VarDecl parseVarDecl() {
        int    line = peek().line;
        String type = parseValueType();
        String name = expect(TokenType.IDENT).value;
        expect(TokenType.EQ);
        AST.Expr init = parseExpr();
        return new AST.VarDecl(type, name, init, line);
    }

    // ─────────────────────────────────────────────
    // <func_decl>
    // ─────────────────────────────────────────────
    private AST.FuncDecl parseFuncDecl() {
        int line = peek().line;
        expect(TokenType.FUNC);
        String name = expect(TokenType.IDENT).value;
        expect(TokenType.LPAREN);

        List<AST.Param> params = new ArrayList<>();
        if (!check(TokenType.RPAREN)) {
            params.add(parseParam());
            while (check(TokenType.COMMA)) {
                advance();
                params.add(parseParam());
            }
        }

        expect(TokenType.RPAREN);
        expect(TokenType.ARROW);
        String returnType = parseType();
        expect(TokenType.LBRACE);
        List<AST.Node> body = parseStmtList();
        expect(TokenType.RBRACE);

        return new AST.FuncDecl(name, params, returnType, body, line);
    }

    /** IDENT ":" <value_type> */
    private AST.Param parseParam() {
        String pname = expect(TokenType.IDENT).value;
        expect(TokenType.COLON);
        String ptype = parseValueType();
        return new AST.Param(pname, ptype);
    }

    // ─────────────────────────────────────────────
    // <stmt_section> ::= { <stmt> }
    // ─────────────────────────────────────────────
    private List<AST.Node> parseStmtSection() {
        return parseStmtList();
    }

    /** Parse statements until we hit EOF or a closing brace. */
    private List<AST.Node> parseStmtList() {
        List<AST.Node> stmts = new ArrayList<>();
        while (!check(TokenType.EOF) && !check(TokenType.RBRACE)) {
            stmts.add(parseStmt());
        }
        return stmts;
    }

    // ─────────────────────────────────────────────
    // <stmt>
    // ─────────────────────────────────────────────
    private AST.Node parseStmt() {
        switch (peek().type) {
            case CURVE:      return parseCurve();
            case AVERAGE:    return parseAverage();
            case RANK:       return parseRank();
            case TRANSCRIPT: return parseTranscript();
            case IF:         return parseIfStmt();
            case FOR:        return parseForStmt();
            case PRINT:      return parsePrintStmt();
            case RETURN:     return parseReturnStmt();
            // var_decl starts with a type keyword
            case INT_TYPE:
            case FLOAT_TYPE:
            case STRING_TYPE:
            case BOOL_TYPE:
                return parseVarDecl();
            // IDENT "(" → call statement
            // IDENT "=" → assignment statement
            case IDENT:
                if (tokens.get(pos + 1).type == TokenType.LPAREN) {
                    return parseCallStmt();
                }
                return parseAssignStmt();
            default:
                throw error("unexpected token '" + peek().value
                        + "' — expected a statement");
        }
    }

    // ── Operations ──

    /** "curve" IDENT "by" <expr> */
    private AST.CurveStmt parseCurve() {
        int line = peek().line;
        expect(TokenType.CURVE);
        String group = expect(TokenType.IDENT).value;
        expect(TokenType.BY);
        AST.Expr amount = parseExpr();
        return new AST.CurveStmt(group, amount, line);
    }

    /** "average" IDENT */
    private AST.AverageStmt parseAverage() {
        int line = peek().line;
        expect(TokenType.AVERAGE);
        String group = expect(TokenType.IDENT).value;
        return new AST.AverageStmt(group, line);
    }

    /** "rank" IDENT */
    private AST.RankStmt parseRank() {
        int line = peek().line;
        expect(TokenType.RANK);
        String group = expect(TokenType.IDENT).value;
        return new AST.RankStmt(group, line);
    }

    /** "transcript" IDENT */
    private AST.TranscriptStmt parseTranscript() {
        int line = peek().line;
        expect(TokenType.TRANSCRIPT);
        String student = expect(TokenType.IDENT).value;
        return new AST.TranscriptStmt(student, line);
    }

    // ── Control flow ──

    /**
     * <if_stmt> ::= "if" <expr> "{" {<stmt>} "}"
     *               { "else if" <expr> "{" {<stmt>} "}" }
     *               [ "else" "{" {<stmt>} "}" ]
     *
     * "else if" is lexed as ELSE then IF — the parser assembles the chain.
     */
    private AST.IfStmt parseIfStmt() {
        int line = peek().line;
        List<AST.Expr>       conditions = new ArrayList<>();
        List<List<AST.Node>> branches   = new ArrayList<>();
        List<AST.Node>       elseBranch = null;

        expect(TokenType.IF);
        conditions.add(parseExpr());
        expect(TokenType.LBRACE);
        branches.add(parseStmtList());
        expect(TokenType.RBRACE);

        while (check(TokenType.ELSE)) {
            advance(); // consume 'else'
            if (check(TokenType.IF)) {
                advance(); // consume 'if'  →  this is an else-if branch
                conditions.add(parseExpr());
                expect(TokenType.LBRACE);
                branches.add(parseStmtList());
                expect(TokenType.RBRACE);
            } else {
                // plain else
                expect(TokenType.LBRACE);
                elseBranch = parseStmtList();
                expect(TokenType.RBRACE);
                break;
            }
        }

        return new AST.IfStmt(conditions, branches, elseBranch, line);
    }

    /**
     * <for_stmt> ::= "for" "student" IDENT "in" IDENT "{" {<stmt>} "}"
     * The keyword "student" is a TokenType.STUDENT token.
     */
    private AST.ForStmt parseForStmt() {
        int line = peek().line;
        expect(TokenType.FOR);
        expect(TokenType.STUDENT);              // mandatory 'student' keyword
        String loopVar   = expect(TokenType.IDENT).value;
        expect(TokenType.IN);
        String groupName = expect(TokenType.IDENT).value;
        expect(TokenType.LBRACE);
        List<AST.Node> body = parseStmtList();
        expect(TokenType.RBRACE);
        return new AST.ForStmt(loopVar, groupName, body, line);
    }

    /** "print" <expr> */
    private AST.PrintStmt parsePrintStmt() {
        int line = peek().line;
        expect(TokenType.PRINT);
        AST.Expr value = parseExpr();
        return new AST.PrintStmt(value, line);
    }

    /** "return" <expr> */
    private AST.ReturnStmt parseReturnStmt() {
        int line = peek().line;
        expect(TokenType.RETURN);
        AST.Expr value = parseExpr();
        return new AST.ReturnStmt(value, line);
    }

    /** IDENT "(" [<arg_list>] ")"  used as a statement (void function call). */
    private AST.CallStmt parseCallStmt() {
        int    line = peek().line;
        String name = expect(TokenType.IDENT).value;
        AST.CallExpr call = parseCallTail(name, line);
        return new AST.CallStmt(call, line);
    }

    /**
     *   var_decl  → starts with type keyword (int, float, string, bool)
     *   assign    → starts with IDENT
     * One token of lookahead suffices — no backtracking needed.
     */
    private AST.AssignStmt parseAssignStmt() {
        int    line = peek().line;
        String name = expect(TokenType.IDENT).value;
        expect(TokenType.EQ);
        AST.Expr value = parseExpr();
        return new AST.AssignStmt(name, value, line);
    }

    // ─────────────────────────────────────────────
    // Expression grammar — 7 precedence layers
    // All binary operators are left-associative (repetition form).
    // Tightest → loosest: unary, * /, + -, < <= > >=, == !=, &&, ||
    // ─────────────────────────────────────────────

    private AST.Expr parseExpr()            { return parseOrExpr(); }

    /** <or_expr> ::= <and_expr> { "||" <and_expr> } */
    private AST.Expr parseOrExpr() {
        AST.Expr left = parseAndExpr();
        while (check(TokenType.OR)) {
            int line = peek().line;
            String op = advance().value;
            left = new AST.BinaryExpr(left, op, parseAndExpr(), line);
        }
        return left;
    }

    /** <and_expr> ::= <equality_expr> { "&&" <equality_expr> } */
    private AST.Expr parseAndExpr() {
        AST.Expr left = parseEqualityExpr();
        while (check(TokenType.AND)) {
            int line = peek().line;
            String op = advance().value;
            left = new AST.BinaryExpr(left, op, parseEqualityExpr(), line);
        }
        return left;
    }

    /** <equality_expr> ::= <relational_expr> { ("==" | "!=") <relational_expr> } */
    private AST.Expr parseEqualityExpr() {
        AST.Expr left = parseRelationalExpr();
        while (check(TokenType.EQ_EQ) || check(TokenType.BANG_EQ)) {
            int line = peek().line;
            String op = advance().value;
            left = new AST.BinaryExpr(left, op, parseRelationalExpr(), line);
        }
        return left;
    }

    /** <relational_expr> ::= <additive_expr> { ("<" | "<=" | ">" | ">=") <additive_expr> } */
    private AST.Expr parseRelationalExpr() {
        AST.Expr left = parseAdditiveExpr();
        while (check(TokenType.LT) || check(TokenType.LT_EQ)
            || check(TokenType.GT) || check(TokenType.GT_EQ)) {
            int line = peek().line;
            String op = advance().value;
            left = new AST.BinaryExpr(left, op, parseAdditiveExpr(), line);
        }
        return left;
    }

    /** <additive_expr> ::= <mult_expr> { ("+" | "-") <mult_expr> } */
    private AST.Expr parseAdditiveExpr() {
        AST.Expr left = parseMultExpr();
        while (check(TokenType.PLUS) || check(TokenType.MINUS)) {
            int line = peek().line;
            String op = advance().value;
            left = new AST.BinaryExpr(left, op, parseMultExpr(), line);
        }
        return left;
    }

    /** <mult_expr> ::= <unary_expr> { ("*" | "/") <unary_expr> } */
    private AST.Expr parseMultExpr() {
        AST.Expr left = parseUnaryExpr();
        while (check(TokenType.STAR) || check(TokenType.SLASH)) {
            int line = peek().line;
            String op = advance().value;
            left = new AST.BinaryExpr(left, op, parseUnaryExpr(), line);
        }
        return left;
    }

    /**
     * <unary_expr> ::= "-" <unary_expr>
     *                | "!" <unary_expr>
     *                | <primary>
     * Right-recursive — allows --x and !!b.
     */
    private AST.Expr parseUnaryExpr() {
        if (check(TokenType.MINUS)) {
            int line = peek().line;
            String op = advance().value;
            return new AST.UnaryExpr(op, parseUnaryExpr(), line);
        }
        if (check(TokenType.BANG)) {
            int line = peek().line;
            String op = advance().value;
            return new AST.UnaryExpr(op, parseUnaryExpr(), line);
        }
        return parsePrimary();
    }

    /**
     * <primary> ::= INT | FLOAT | STRING | "true" | "false"
     *             | IDENT | <func_call_expr> | "(" <expr> ")"
     * IDENT vs func_call: look ahead for "(" to distinguish.
     */
    private AST.Expr parsePrimary() {
        Token t = peek();

        switch (t.type) {
            case INT:
                advance();
                return new AST.IntLiteral(Integer.parseInt(t.value), t.line);

            case FLOAT:
                advance();
                return new AST.FloatLiteral(Double.parseDouble(t.value), t.line);

            case STRING:
                advance();
                return new AST.StringLiteral(t.value, t.line);

            case TRUE:
                advance();
                return new AST.BoolLiteral(true, t.line);

            case FALSE:
                advance();
                return new AST.BoolLiteral(false, t.line);

            case IDENT:
                advance();
                // Peek ahead: if next token is '(' this is a function call
                if (check(TokenType.LPAREN)) {
                    return parseCallTail(t.value, t.line);
                }
                return new AST.IdentExpr(t.value, t.line);

            case LPAREN:
                advance();
                AST.Expr inner = parseExpr();
                expect(TokenType.RPAREN);
                return inner;

            default:
                throw error("expected an expression but found '" + t.value + "'");
        }
    }

    /**
     * Called after IDENT is consumed and '(' has been detected.
     * Parses "(" [ <arg_list> ] ")"
     */
    private AST.CallExpr parseCallTail(String name, int line) {
        expect(TokenType.LPAREN);
        List<AST.Expr> args = new ArrayList<>();

        if (!check(TokenType.RPAREN)) {
            args.add(parseExpr());
            while (check(TokenType.COMMA)) {
                advance();
                args.add(parseExpr());
            }
        }

        expect(TokenType.RPAREN);
        return new AST.CallExpr(name, args, line);
    }

    // ─────────────────────────────────────────────
    // Type Helpers
    // ─────────────────────────────────────────────

    /** <type> ::= <value_type> | "void" */
    private String parseType() {
        if (check(TokenType.VOID)) { advance(); return "void"; }
        return parseValueType();
    }

    /** <value_type> ::= "int" | "float" | "string" | "bool" */
    private String parseValueType() {
        switch (peek().type) {
            case INT_TYPE:    advance(); return "int";
            case FLOAT_TYPE:  advance(); return "float";
            case STRING_TYPE: advance(); return "string";
            case BOOL_TYPE:   advance(); return "bool";
            default:
                throw error("expected a type (int, float, string, bool) but found '"
                        + peek().value + "'");
        }
    }

    // ─────────────────────────────────────────────
    // Token Navigation Helpers
    // ─────────────────────────────────────────────
    private Token peek() {
        return tokens.get(pos);
    }

    private Token advance() {
        Token t = tokens.get(pos);
        if (t.type != TokenType.EOF) pos++;
        return t;
    }

    private boolean check(TokenType type) {
        return peek().type == type;
    }

    /**
     * Consume the next token if it matches type; otherwise throw a parse error
     * with the current line number.
     */
    private Token expect(TokenType type) {
        Token t = peek();
        if (t.type != type) {
            throw error("expected " + describe(type)
                    + " but found '" + t.value + "'");
        }
        return advance();
    }

    /** Human-readable name for a token type used in error messages. */
    private String describe(TokenType type) {
        switch (type) {
            case IDENT:        return "identifier";
            case INT:          return "integer literal";
            case FLOAT:        return "float literal";
            case STRING:       return "string literal";
            case LBRACE:       return "'{'";
            case RBRACE:       return "'}'";
            case LPAREN:       return "'('";
            case RPAREN:       return "')'";
            case LBRACKET:     return "'['";
            case RBRACKET:     return "']'";
            case COMMA:        return "','";
            case COLON:        return "':'";
            case EQ:           return "'='";
            case ARROW:        return "'->'";
            case BY:           return "'by'";
            case IN:           return "'in'";
            case KW_ID:        return "'id'";
            case KW_FIRST_NAME:return "'firstName'";
            case KW_LAST_NAME: return "'lastName'";
            case KW_GRADES:    return "'grades'";
            case STUDENT:      return "'student'";
            case EOF:          return "end of file";
            default:           return "'" + type.toString().toLowerCase() + "'";
        }
    }

    private ParseException error(String message) {
        return new ParseException("Parse error at line " + peek().line + ": " + message);
    }

    // ─────────────────────────────────────────────
    // Exception
    // ─────────────────────────────────────────────
    public static class ParseException extends RuntimeException {
        public ParseException(String message) { super(message); }
    }
}
