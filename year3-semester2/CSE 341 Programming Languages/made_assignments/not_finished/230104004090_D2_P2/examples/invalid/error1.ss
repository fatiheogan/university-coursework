// ─────────────────────────────────────────────
// ScoreScript Malformed Program 1 — error1.ss
// Expected error: wrong field order in student block
// lastName appears before firstName — violates fixed mandatory order
// CSE 341 · Spring 2026
// ─────────────────────────────────────────────

student s1 {
    id: "230104004090",
    lastName: "OGAN",
    firstName: "Fatih Emre",
    grades: { math: 85, science: 90 }
}

group ClassZ23 = [s1]

transcript s1
