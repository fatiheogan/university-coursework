// error_type.ss
// CSE 341 · ScoreScript · D3 Part 2 — Type error example
//
// This is a syntactically valid ScoreScript program: it parses cleanly.
// It fails the STATIC type checker, before any execution, because it tries
// to capture the result of a void function in a string variable.
//
// classReport(...) is declared "-> void": it prints a report and returns
// nothing. ScoreScript has no implicit conversions and 'void' is only a
// return type, never a value, so binding its result to a variable is
// rejected at type-check time with a line number — the program never runs.

student s1 {
    id: "230104004090",
    firstName: "Fatih Emre",
    lastName: "OGAN",
    grades: { math: 85, science: 90, history: 72 }
}
student s2 {
    id: "230104004091",
    firstName: "Ayse",
    lastName: "KURT",
    grades: { math: 91, science: 88, history: 95 }
}

group ClassZ23 = [s1, s2]

int curveAmount = 5

func classReport(threshold: int) -> void {
    print "=== Class Report ==="
    transcript s1
    transcript s2
    if threshold > 60 {
        print "Threshold is acceptable"
    }
    else {
        print "Threshold is low"
    }
}

// BUG: classReport returns void. Its result cannot be a value, so it
// cannot initialize a string (or any) variable. The type checker stops
// here and reports the line.
string summary = classReport(curveAmount)

curve ClassZ23 by curveAmount
average ClassZ23
rank ClassZ23
