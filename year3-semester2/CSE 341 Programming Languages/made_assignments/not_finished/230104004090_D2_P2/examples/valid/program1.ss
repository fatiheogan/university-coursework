// ─────────────────────────────────────────────
// ScoreScript Example Program 1 — program1.ss
// Focus: Student declarations, groups,
//        variable declarations, curve / average / rank / transcript
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
    id: "230104004093",
    firstName: "Zeynep",
    lastName: "ARSLAN",
    grades: { math: 45, science: 55, history: 50 }
}

group ClassZ23 = [s1, s2, s3, s4]
group TopStudents = [s1, s2]

int curveAmount = 5
int passingThreshold = 60

// ── Statements ──

// Apply a curve of 5 points to the whole class
curve ClassZ23 by curveAmount

// Show the class average after curving
average ClassZ23

// Rank all students by overall average
rank ClassZ23

// Print individual transcripts for each student
transcript s1
transcript s2
transcript s3
transcript s4

// Also show the top group average separately
average TopStudents
rank TopStudents
