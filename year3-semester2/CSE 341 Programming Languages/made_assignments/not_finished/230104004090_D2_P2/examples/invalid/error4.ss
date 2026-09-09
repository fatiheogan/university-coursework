// ─────────────────────────────────────────────
// ScoreScript Malformed Program 4 — error4.ss
// Expected error: missing expression after 'by' in curve statement
// curve requires a valid expression after 'by'
// CSE 341 · Spring 2026
// ─────────────────────────────────────────────

student s1 {
    id: "230104004090",
    firstName: "Fatih Emre",
    lastName: "OGAN",
    grades: { math: 85, science: 90 }
}

group ClassZ23 = [s1]

curve ClassZ23 by

average ClassZ23
