// E3 test 4 — NEGATIVE CONTROL: a fully well-typed program.
// This is the counterpart to tests 1-3. It deliberately exercises the
// SAME checker machinery those three tests trip:
//   - an int declaration AND a float declaration (test 1's path, but correct)
//   - a void function called as a STATEMENT, result not captured (test 2's
//     path, but used legally)
//   - a value-returning function called with the CORRECT arity and arg
//     type, its result assigned to a matching variable (test 3's path,
//     but correct)
// Expected result: the type checker reports NO error and the program runs.
// If this produced a type error, it would mean the checker is over-
// rejecting, which tests 1-3 alone could not reveal.

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
float passThreshold = 60.0

func letterGrade(avg: float) -> string {
    if avg >= 90.0 { return "AA" }
    else if avg >= 85.0 { return "BA" }
    else if avg >= 60.0 { return "CC" }
    else { return "FF" }
}

func report(label: string) -> void {
    print label
    transcript s1
    transcript s2
}

report("=== Class Report ===")

curve ClassZ23 by curveAmount
average ClassZ23
rank ClassZ23

float topAvg = 88.5
string grade1 = letterGrade(topAvg)
print grade1

for student s in ClassZ23 {
    transcript s
}
