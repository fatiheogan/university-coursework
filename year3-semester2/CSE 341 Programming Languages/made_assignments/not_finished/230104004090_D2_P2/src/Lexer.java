import java.util.ArrayList;
import java.util.List;

/**
 * ScoreScript Lexer
 * Tokenizes ScoreScript source code and returns a list of tokens with line numbers.
 * Comment syntax: // single-line only.
 * CSE 341 · Spring 2026
 */
public class Lexer {

    // ─────────────────────────────────────────────
    // State
    // ─────────────────────────────────────────────
    private final String source;
    private int          pos;
    private int          line;

    public Lexer(String source) {
        this.source = source;
        this.pos    = 0;
        this.line   = 1;
    }

    // ─────────────────────────────────────────────
    // Public Entry Point
    // ─────────────────────────────────────────────
    public List<Token> tokenize() {
        List<Token> tokens = new ArrayList<>();

        while (!isAtEnd()) {
            skipWhitespace();
            if (isAtEnd()) break;
            Token t = nextToken();
            if (t != null) tokens.add(t);
        }

        tokens.add(new Token(TokenType.EOF, "", line));
        return tokens;
    }

    // ─────────────────────────────────────────────
    // Core Tokenizer
    // ─────────────────────────────────────────────
    private Token nextToken() {
        int  tokenLine = line;
        char c         = advance();

        switch (c) {
            // ── Separators ──
            case '{': return new Token(TokenType.LBRACE,   "{", tokenLine);
            case '}': return new Token(TokenType.RBRACE,   "}", tokenLine);
            case '(': return new Token(TokenType.LPAREN,   "(", tokenLine);
            case ')': return new Token(TokenType.RPAREN,   ")", tokenLine);
            case '[': return new Token(TokenType.LBRACKET, "[", tokenLine);
            case ']': return new Token(TokenType.RBRACKET, "]", tokenLine);
            case ',': return new Token(TokenType.COMMA,    ",", tokenLine);
            case ':': return new Token(TokenType.COLON,    ":", tokenLine);

            // ── Arithmetic ──
            case '+': return new Token(TokenType.PLUS,  "+", tokenLine);
            case '*': return new Token(TokenType.STAR,  "*", tokenLine);

            // ── / or // (comment) ──
            case '/':
                if (match('/')) {
                    while (!isAtEnd() && peek() != '\n') advance();
                    return null; // comment — tokenize() will continue
                }
                return new Token(TokenType.SLASH, "/", tokenLine);

            // ── - or -> ──
            case '-':
                if (match('>')) return new Token(TokenType.ARROW,   "->", tokenLine);
                return             new Token(TokenType.MINUS,       "-",  tokenLine);

            // ── = or == ──
            case '=':
                if (match('=')) return new Token(TokenType.EQ_EQ,   "==", tokenLine);
                return             new Token(TokenType.EQ,          "=",  tokenLine);

            // ── ! or != ──
            case '!':
                if (match('=')) return new Token(TokenType.BANG_EQ, "!=", tokenLine);
                return             new Token(TokenType.BANG,        "!",  tokenLine);

            // ── < or <= ──
            case '<':
                if (match('=')) return new Token(TokenType.LT_EQ,   "<=", tokenLine);
                return             new Token(TokenType.LT,          "<",  tokenLine);

            // ── > or >= ──
            case '>':
                if (match('=')) return new Token(TokenType.GT_EQ,   ">=", tokenLine);
                return             new Token(TokenType.GT,          ">",  tokenLine);

            // ── && ──
            case '&':
                if (match('&')) return new Token(TokenType.AND, "&&", tokenLine);
                throw error("unexpected character '&' — did you mean '&&'?", tokenLine);

            // ── || ──
            case '|':
                if (match('|')) return new Token(TokenType.OR, "||", tokenLine);
                throw error("unexpected character '|' — did you mean '||'?", tokenLine);

            // ── String literal ──
            case '"':
                return readString(tokenLine);

            default:
                if (Character.isDigit(c))               return readNumber(c, tokenLine);
                if (Character.isLetter(c) || c == '_')  return readWord(c, tokenLine);
                throw error("unexpected character '" + c + "'", tokenLine);
        }
    }

    // ─────────────────────────────────────────────
    // Readers
    // ─────────────────────────────────────────────

    /**
     * Read a string literal. Opening '"' already consumed.
     * Supports escape sequences: \" \\ \n \t
     */
    private Token readString(int tokenLine) {
        StringBuilder sb = new StringBuilder();

        while (!isAtEnd() && peek() != '"') {
            char c = advance();
            if (c == '\n') {
                throw error("unterminated string — newline inside string literal", tokenLine);
            }
            if (c == '\\') {
                if (isAtEnd()) break;
                char esc = advance();
                switch (esc) {
                    case '"':  sb.append('"');  break;
                    case '\\': sb.append('\\'); break;
                    case 'n':  sb.append('\n'); break;
                    case 't':  sb.append('\t'); break;
                    default:
                        throw error("unknown escape sequence '\\" + esc + "'", tokenLine);
                }
            } else {
                sb.append(c);
            }
        }

        if (isAtEnd()) {
            throw error("unterminated string — missing closing '\"'", tokenLine);
        }

        advance(); // consume closing '"'
        return new Token(TokenType.STRING, sb.toString(), tokenLine);
    }

    /**
     * Read INT or FLOAT literal. First digit already consumed.
     * FLOAT is tried before INT: looks ahead for '.' followed by a digit.
     * This prevents 60.0 being tokenized as INT(60) DOT INT(0).
     */
    private Token readNumber(char first, int tokenLine) {
        StringBuilder sb = new StringBuilder();
        sb.append(first);

        while (!isAtEnd() && Character.isDigit(peek())) {
            sb.append(advance());
        }

        // Attempt float: current token so far + '.' + digit(s)
        if (!isAtEnd() && peek() == '.'
                && (pos + 1) < source.length()
                && Character.isDigit(source.charAt(pos + 1))) {
            sb.append(advance()); // consume '.'
            while (!isAtEnd() && Character.isDigit(peek())) {
                sb.append(advance());
            }
            return new Token(TokenType.FLOAT, sb.toString(), tokenLine);
        }

        return new Token(TokenType.INT, sb.toString(), tokenLine);
    }

    /**
     * Read keyword or identifier. First char already consumed.
     */
    private Token readWord(char first, int tokenLine) {
        StringBuilder sb = new StringBuilder();
        sb.append(first);

        while (!isAtEnd() && (Character.isLetterOrDigit(peek()) || peek() == '_')) {
            sb.append(advance());
        }

        String    word = sb.toString();
        TokenType type = keyword(word);
        return new Token(type, word, tokenLine);
    }

    // ─────────────────────────────────────────────
    // Keyword Table
    // ─────────────────────────────────────────────
    private TokenType keyword(String w) {
        switch (w) {
            // Statement / declaration
            case "student":    return TokenType.STUDENT;
            case "group":      return TokenType.GROUP;
            case "func":       return TokenType.FUNC;
            case "for":        return TokenType.FOR;
            case "in":         return TokenType.IN;
            case "if":         return TokenType.IF;
            case "else":       return TokenType.ELSE;
            case "return":     return TokenType.RETURN;
            case "print":      return TokenType.PRINT;
            case "by":         return TokenType.BY;
            // Operations
            case "curve":      return TokenType.CURVE;
            case "average":    return TokenType.AVERAGE;
            case "rank":       return TokenType.RANK;
            case "transcript": return TokenType.TRANSCRIPT;
            // Types
            case "int":        return TokenType.INT_TYPE;
            case "float":      return TokenType.FLOAT_TYPE;
            case "string":     return TokenType.STRING_TYPE;
            case "bool":       return TokenType.BOOL_TYPE;
            case "void":       return TokenType.VOID;
            // Boolean literals
            case "true":       return TokenType.TRUE;
            case "false":      return TokenType.FALSE;
            // Student field keywords
            case "id":         return TokenType.KW_ID;
            case "firstName":  return TokenType.KW_FIRST_NAME;
            case "lastName":   return TokenType.KW_LAST_NAME;
            case "grades":     return TokenType.KW_GRADES;
            // User-defined identifier
            default:           return TokenType.IDENT;
        }
    }

    // ─────────────────────────────────────────────
    // Whitespace
    // ─────────────────────────────────────────────
    private void skipWhitespace() {
        while (!isAtEnd()) {
            char c = peek();
            if      (c == ' ' || c == '\r' || c == '\t') advance();
            else if (c == '\n') { line++; advance(); }
            else break;
        }
    }

    // ─────────────────────────────────────────────
    // Primitive Helpers
    // ─────────────────────────────────────────────
    private boolean isAtEnd()           { return pos >= source.length(); }
    private char    peek()              { return source.charAt(pos); }
    private char    advance()           { return source.charAt(pos++); }

    private boolean match(char expected) {
        if (isAtEnd() || source.charAt(pos) != expected) return false;
        pos++;
        return true;
    }

    private LexerException error(String msg, int errorLine) {
        return new LexerException("Lexer error at line " + errorLine + ": " + msg);
    }

    // ─────────────────────────────────────────────
    // Exception
    // ─────────────────────────────────────────────
    public static class LexerException extends RuntimeException {
        public LexerException(String message) { super(message); }
    }
}
