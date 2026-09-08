def BruteForce(a, n):
    result = 1
    for i in range(n):
        result = result * a
    return result


def DivideAndConquer(a, n):
    if(a == 0):
        return 0
    if(n == 0):
        return 1
    #if even
    elif(n % 2 == 0):
        return DivideAndConquer(a, n/2) * DivideAndConquer(a, n/2)
    #if odd
    else:
        return a * DivideAndConquer(a, n-1)


def main():

    a = 3
    n = 4
    print(BruteForce(a, n))
    print(DivideAndConquer(a, n))

main()