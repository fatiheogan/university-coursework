# ScoreScript

A domain-specific language for defining students, grouping them, and running grade operations.
Designed and implemented for CSE 341 — Concepts of Programming Languages, Spring 2026, Gebze Technical University.

---

## Project Structure

```
ScoreScript/
├── src/                    # Java source files
│   ├── TokenType.java
│   ├── Token.java
│   ├── Lexer.java
│   ├── AST.java
│   ├── Parser.java
│   ├── TypeChecker.java
│   ├── Interpreter.java
│   └── Main.java
├── examples/
│   ├── valid/              # Programs that type-check and execute
│   │   ├── program1.ss
│   │   ├── program2.ss
│   │   └── program3.ss
│   └── invalid/            # Programs that trigger errors
│       ├── error1.ss       # Parse errors
│       ├── error2.ss
│       ├── error3.ss
│       ├── error4.ss
│       ├── error5.ss
│       └── error_type.ss   # Type error (Part 2)
├── out/                    # Compiled .class files (created by build step)
├── Makefile
└── README.md
```

---

## Requirements

- Java 11 or later
- make (available via Git Bash on Windows, or natively on Linux/macOS)

Verify your Java version:

```bash
java -version
```

---

## Build & Run (with Makefile)

All commands are run from the project root using Git Bash (Windows) or any terminal (Linux/macOS).

### Compile

```bash
make
```

### Run a program

```bash
make run FILE=examples/valid/program1.ss
```

### Dump the AST

```bash
make ast FILE=examples/valid/program1.ss
```

### Run all tests (valid + invalid)

```bash
make test
```

### Clean compiled files

```bash
make clean
```

---

## Build & Run (without Makefile)

If make is not available, use these commands directly:

### Compile

```bash
mkdir out
javac -d out src/TokenType.java src/Token.java src/Lexer.java src/AST.java src/Parser.java src/TypeChecker.java src/Interpreter.java src/Main.java
```

### Run

```bash
java -cp out Main <source_file>
java -cp out Main <source_file> --dump-ast
```

### Exit codes

| Code | Meaning |
|------|---------|
| `0`  | Success, program type-checked and executed |
| `1`  | Lexer or parser error |
| `2`  | Usage error or file could not be read |
| `3`  | Type error, static type check failed |
| `4`  | Runtime error, interpreter aborted |

---

## Examples

### Valid programs

**program1.ss** — Student declarations, groups, variable declarations, all four operations:

```bash
make run FILE=examples/valid/program1.ss
# Average of ClassZ23: 77.17
# Ranking of ClassZ23:
#   1. Ayse KURT (96.33)
#   2. Fatih Emre OGAN (87.33)
#   3. Mehmet DEMIR (70.00)
#   4. Zeynep ARSLAN (55.00)
# Transcript for 230104004090 - Fatih Emre OGAN
#   math: 90
#   science: 95
#   history: 77
#   Average: 87.33
# ... (transcripts for s2, s3, s4 follow)
# Average of TopStudents: 91.83
# Ranking of TopStudents:
#   1. Ayse KURT (96.33)
#   2. Fatih Emre OGAN (87.33)
```

**program2.ss** — Functions with if / else if / else chains, bool variables, print statements:

```bash
make run FILE=examples/valid/program2.ss
# Transcript for 230104004090 - Fatih Emre OGAN
# ... (transcripts for s1, s2, s3)
# Average of SectionA: 77.22
# Average of SectionA: 80.11
# Ranking of SectionA:
#   1. Burak SAHIN (97.67)
#   2. Fatih Emre OGAN (87.33)
#   3. Selin YILMAZ (55.33)
# true
# false
# BA
# CC
# Status: HONOR STUDENT
# Status: PASSING
# Status: FAILING
```

**program3.ss** — For loops, assignment statements, expressions in curve, multiple groups:

```bash
make run FILE=examples/valid/program3.ss
# Transcript for 230104004090 - Fatih Emre OGAN
# ... (transcripts for all five students via the for-loop)
# Average of MorningClass: 79.56
# Ranking of MorningClass:
#   1. Ayse KURT (91.33)
#   2. Fatih Emre OGAN (82.33)
#   3. Mehmet DEMIR (65.00)
# Average of AfternoonClass: 57.50
# ... (further rankings, transcripts after curve, and a second AfternoonClass curve)
# Average of AllStudents: 79.20
# Ranking of AllStudents:
#   1. Ayse KURT (97.67)
#   2. Elif CELIK (91.00)
#   3. Fatih Emre OGAN (89.33)
#   4. Mehmet DEMIR (72.00)
#   5. Deniz KAPLAN (46.00)
```

---

### Invalid programs

The first five programs trigger parse errors; `error_type.ss` parses successfully but is rejected by the type checker before execution. Every error message includes a line number.

| File | Error | Expected message | Exit code |
|------|-------|------------------|-----------|
| `error1.ss` | Wrong field order in student block | `Parse error at line 10: expected 'firstName' but found 'lastName'` | `1` |
| `error2.ss` | `:` instead of `=` in group declaration | `Parse error at line 22: expected '=' but found ':'` | `1` |
| `error3.ss` | Missing `->` in func declaration | `Parse error at line 17: expected '->' but found 'string'` | `1` |
| `error4.ss` | Missing expression after `by` in curve | `Parse error at line 19: expected an expression but found 'average'` | `1` |
| `error5.ss` | Statement appears before declaration section ends | `Parse error at line 18: unexpected token 'student' ? expected a statement` | `1` |
| `error_type.ss` | Void function result used as a value | `Type error at line 45: cannot use the result of void function 'classReport' as a value` | `3` |

---

## Language Quick Reference

A ScoreScript program has two sections — all declarations must come before all statements:

```
program = decl_section stmt_section
```

### Student declaration

Fields are mandatory and must appear in this exact order:

```
student s1 {
    id: "230104004090",
    firstName: "Fatih Emre",
    lastName: "OGAN",
    grades: { math: 85, science: 90, history: 72 }
}
```

### Group declaration

```
group ClassZ23 = [s1, s2, s3]
```

### Variable declaration

```
int    curveAmount = 5
float  threshold   = 59.5
string label       = "passing"
bool   applied     = false
```

### Function declaration

```
func letterGrade(avg: float) -> string {
    if avg >= 90.0 { return "AA" }
    else if avg >= 85.0 { return "BA" }
    else { return "FF" }
}

func printAll() -> void {
    transcript s1
}
```

### Operations

```
curve      ClassZ23 by 5                 // add points to every grade, cap at 100
curve      ClassZ23 by curveAmount + 3   // expression allowed after by
average    ClassZ23                      // print overall group average
rank       ClassZ23                      // rank students by overall average
transcript s1                            // print all grades for one student
```

### Control flow

```
if threshold < 60.0 {
    curve ClassZ23 by 10
}
else if threshold < 70.0 {
    curve ClassZ23 by 5
}
else {
    rank ClassZ23
}
```

### For loop

Iterates over every student in a group. The `student` keyword is mandatory:

```
for student s in ClassZ23 {
    transcript s
}
```

### Comments

Single-line only, using `//`:

```
// This is a comment
curve ClassZ23 by 5   // inline comment
```

---

## Token Summary

| Category | Tokens |
|----------|--------|
| Declaration keywords | `student` `group` `func` `for` `in` `if` `else` `return` `print` `by` |
| Operation keywords | `curve` `average` `rank` `transcript` |
| Type keywords | `int` `float` `string` `bool` `void` |
| Student field keywords | `id` `firstName` `lastName` `grades` |
| Literals | `INT` `FLOAT` `STRING` `true` `false` |
| Operators | `+` `-` `*` `/` `!` `=` `==` `!=` `<` `<=` `>` `>=` `&&` `\|\|` `->` |
| Separators | `{` `}` `(` `)` `[` `]` `,` `:` |

---

## Operator Precedence

From tightest to loosest binding:

| Level | Operator(s) | Associativity |
|-------|------------|---------------|
| 1 | `-` `!` (unary) | Right |
| 2 | `*` `/` | Left |
| 3 | `+` `-` | Left |
| 4 | `<` `<=` `>` `>=` | Left |
| 5 | `==` `!=` | Left |
| 6 | `&&` | Left |
| 7 | `\|\|` | Left |
