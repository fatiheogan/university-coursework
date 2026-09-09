// ─────────────────────────────────────────────
// ScoreScript Malformed Program 3 — error3.ss
// Expected error: missing '->' between parameter list and return type
// func declaration requires '->' before the return type
// CSE 341 · Spring 2026
// ─────────────────────────────────────────────

student s1 {
    id: "230104004090",
    firstName: "Fatih Emre",
    lastName: "OGAN",
    grades: { math: 85, science: 90 }
}

group ClassZ23 = [s1]

func letterGrade(avg: float) string {
    if avg >= 90 { return "AA" }
    else if avg >= 85 { return "BA" }
    else { return "FF" }
}

transcript s1
