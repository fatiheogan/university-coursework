// E3 test 3 — function called with the wrong number of arguments
// letterGrade takes exactly one parameter; the call passes two.
// Expected: type error on the call line (arity mismatch).

student s1 {
    id: "230104004090",
    firstName: "Fatih Emre",
    lastName: "OGAN",
    grades: { math: 85, science: 90, history: 72 }
}

group ClassZ23 = [s1]

func letterGrade(avg: float) -> string {
    if avg >= 90.0 { return "AA" }
    else if avg >= 85.0 { return "BA" }
    else if avg >= 60.0 { return "CC" }
    else { return "FF" }
}

float studentAvg = 88.5
string grade1 = letterGrade(studentAvg, 10)

average ClassZ23
