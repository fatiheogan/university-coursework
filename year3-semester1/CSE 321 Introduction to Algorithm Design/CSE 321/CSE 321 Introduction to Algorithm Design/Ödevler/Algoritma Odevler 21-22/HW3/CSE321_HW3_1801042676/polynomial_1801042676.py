def BruteForcePolynomial(x,array):
    result=0
    for i in range(len(array)):
        result = result + (x ** i) * array[i]
    return result

print(BruteForcePolynomial(2,[1,10]))