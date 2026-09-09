// ─────────────────────────────────────────────
// ScoreScript Example Program 2 — program2.ss
// Focus: User-defined functions, if / else if / else chains,
//        float variables, boolean expressions, print statement
// CSE 341 · Spring 2026
// ─────────────────────────────────────────────

// ── Declarations ──

student s1 {
    id: "230104004090",
    firstName: "Fatih Emre",
    lastName: "OGAN",
    grades: { math: 85, science: 90, english: 78 }
}

student s2 {
    id: "230104004094",
    firstName: "Selin",
    lastName: "YILMAZ",
    grades: { math: 42, science: 55, english: 60 }
}

student s3 {
    id: "230104004095",
    firstName: "Burak",
    lastName: "SAHIN",
    grades: { math: 95, science: 98, english: 92 }
}

group SectionA = [s1, s2, s3]

float passingGrade = 59.5
float honorThreshold = 90.0
bool curveApplied = false

// Returns a letter grade string based on numeric average
func letterGrade(avg: float) -> string {
    if avg >= 90.0 { return "AA" }
    else if avg >= 85.0 { return "BA" }
    else if avg >= 75.0 { return "BB" }
    else if avg >= 65.0 { return "CB" }
    else if avg >= 60.0 { return "CC" }
    else { return "FF" }
}

// Returns true if the student passes (avg strictly above passingGrade)
func isPassing(avg: float) -> bool {
    if avg > passingGrade { return true }
    else { return false }
}

// Prints a summary label based on the average
func printLabel(avg: float) -> void {
    if avg >= honorThreshold {
        print "Status: HONOR STUDENT"
    }
    else if avg >= passingGrade {
        print "Status: PASSING"
    }
    else {
        print "Status: FAILING"
    }
}

// ── Statements ──

// Show each student transcript before any curve
transcript s1
transcript s2
transcript s3

// Check whether the class average warrants a curve
average SectionA

// Apply a curve only if needed — decided manually here
if curveApplied == false {
    curve SectionA by 3
    curveApplied = true
}

// After curve: recheck
average SectionA
rank SectionA

// Print passing status using the function
bool pass1 = isPassing(72.0)
bool pass2 = isPassing(55.0)
string grade1 = letterGrade(88.5)
string grade2 = letterGrade(61.0)
print pass1
print pass2
print grade1
print grade2
printLabel(92.0)
printLabel(61.0)
printLabel(45.0)
