// ─────────────────────────────────────────────
// ScoreScript Malformed Program 2 — error2.ss
// Expected error: ':' used instead of '=' in group declaration
// group syntax requires '=' not ':'
// CSE 341 · Spring 2026
// ─────────────────────────────────────────────

student s1 {
    id: "230104004090",
    firstName: "Fatih Emre",
    lastName: "OGAN",
    grades: { math: 85, science: 90 }
}

student s2 {
    id: "230104004091",
    firstName: "Ayse",
    lastName: "KURT",
    grades: { math: 91, science: 88 }
}

group ClassZ23 : [s1, s2]

average ClassZ23
