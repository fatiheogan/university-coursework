public enum TokenType {

    // ── Statement / Declaration Keywords ──
    STUDENT, GROUP, FUNC, FOR, IN, IF, ELSE,
    RETURN, PRINT, BY,

    // ── Operation Keywords ──
    CURVE, AVERAGE, RANK, TRANSCRIPT,

    // ── Type Keywords ──
    INT_TYPE, FLOAT_TYPE, STRING_TYPE, BOOL_TYPE, VOID,

    // ── Student Field Keywords ──
    KW_ID, KW_FIRST_NAME, KW_LAST_NAME, KW_GRADES,

    // ── Boolean Literals ──
    TRUE, FALSE,

    // ── Literals ──
    INT,        // e.g. 85
    FLOAT,      // e.g. 60.0
    STRING,     // e.g. "Fatih Emre"

    // ── Identifier ──
    IDENT,      // e.g. ClassZ23, s1, threshold

    // ── Arithmetic Operators ──
    PLUS,       // +
    MINUS,      // -
    STAR,       // *
    SLASH,      // /

    // ── Unary Operators ──
    BANG,       // !

    // ── Assignment / Comparison ──
    EQ,         // =
    EQ_EQ,      // ==
    BANG_EQ,    // !=
    LT,         // <
    LT_EQ,      // <=
    GT,         // >
    GT_EQ,      // >=

    // ── Logical Operators ──
    AND,        // &&
    OR,         // ||

    // ── Arrow ──
    ARROW,      // ->

    // ── Separators ──
    LBRACE,     // {
    RBRACE,     // }
    LPAREN,     // (
    RPAREN,     // )
    LBRACKET,   // [
    RBRACKET,   // ]
    COMMA,      // ,
    COLON,      // :

    // ── Special ──
    EOF
}
