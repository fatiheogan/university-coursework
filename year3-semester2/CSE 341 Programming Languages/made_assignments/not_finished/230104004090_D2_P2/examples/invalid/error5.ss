// ─────────────────────────────────────────────
// ScoreScript Malformed Program 5 — error5.ss
// Expected error: statement appears before declaration section ends
// ScoreScript requires all declarations before any statements
// CSE 341 · Spring 2026
// ─────────────────────────────────────────────

student s1 {
    id: "230104004090",
    firstName: "Fatih Emre",
    lastName: "OGAN",
    grades: { math: 85, science: 90 }
}

// This curve statement interrupts the declaration section
curve ClassZ23 by 5

student s2 {
    id: "230104004091",
    firstName: "Ayse",
    lastName: "KURT",
    grades: { math: 91, science: 88 }
}

group ClassZ23 = [s1, s2]

average ClassZ23
