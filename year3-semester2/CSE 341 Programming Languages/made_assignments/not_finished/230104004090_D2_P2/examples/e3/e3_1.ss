// E3 test 1 — int variable initialized with a float value
// Expected: type error on the line of the bad declaration.
// Everything else is well-typed so the checker reaches exactly that line.

student s1 {
    id: "230104004090",
    firstName: "Fatih Emre",
    lastName: "OGAN",
    grades: { math: 85, science: 90, history: 72 }
}

group ClassZ23 = [s1]

int passingThreshold = 60
int curveAmount = 3.19

curve ClassZ23 by curveAmount
average ClassZ23
