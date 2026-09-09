// E3 test 2 — using the result of a void function as a value
// Expected: type error on the assignment that captures the void call.
// The function body and all earlier declarations are well-typed.

student s1 {
    id: "230104004090",
    firstName: "Fatih Emre",
    lastName: "OGAN",
    grades: { math: 85, science: 90, history: 72 }
}

group ClassZ23 = [s1]

func printReport(threshold: int) -> void {
    print "=== Report ==="
    transcript s1
    if threshold > 60 {
        print "Threshold OK"
    }
    else {
        print "Threshold low"
    }
}

int limit = 60
string summary = printReport(limit)

average ClassZ23
