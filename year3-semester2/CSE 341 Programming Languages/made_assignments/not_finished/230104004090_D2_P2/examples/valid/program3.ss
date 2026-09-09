// ─────────────────────────────────────────────
// ScoreScript Example Program 3 — program3.ss
// Focus: for loops, assignment statements,
//        expressions in curve, nested if inside for,
//        multiple groups, all four operations used
// CSE 341 · Spring 2026
// ─────────────────────────────────────────────

// ── Declarations ──

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

student s3 {
    id: "230104004092",
    firstName: "Mehmet",
    lastName: "DEMIR",
    grades: { math: 60, science: 70, history: 65 }
}

student s4 {
    id: "230104004096",
    firstName: "Deniz",
    lastName: "KAPLAN",
    grades: { math: 30, science: 40, history: 35 }
}

student s5 {
    id: "230104004097",
    firstName: "Elif",
    lastName: "CELIK",
    grades: { math: 78, science: 82, history: 80 }
}

group MorningClass  = [s1, s2, s3]
group AfternoonClass = [s4, s5]
group AllStudents    = [s1, s2, s3, s4, s5]

int baseCurve = 4
int bonusCurve = 3
float lowAvgCutoff = 65.0

func needsCurve(avg: float) -> bool {
    if avg < lowAvgCutoff { return true }
    else { return false }
}

// ── Statements ──

// Print every student transcript using a for loop
for student s in AllStudents {
    transcript s
}

// Show starting averages and ranks per group
average MorningClass
rank MorningClass
average AfternoonClass
rank AfternoonClass

// Apply a computed curve expression to MorningClass
// baseCurve + bonusCurve = 7 total
curve MorningClass by baseCurve + bonusCurve

// AfternoonClass gets only the base curve
curve AfternoonClass by baseCurve

// Re-rank after curving
rank AllStudents

// Use a for loop with an if inside:
// print a flag for each student individually
for student s in MorningClass {
    transcript s
    print needsCurve(lowAvgCutoff)
}

// Reassign curve variables and apply again conditionally
baseCurve = baseCurve + 1
bonusCurve = 2

if needsCurve(58.0) == true {
    curve AfternoonClass by baseCurve + bonusCurve
    average AfternoonClass
    rank AfternoonClass
}

// Final state of all students
for student s in AllStudents {
    transcript s
}

average AllStudents
rank AllStudents
