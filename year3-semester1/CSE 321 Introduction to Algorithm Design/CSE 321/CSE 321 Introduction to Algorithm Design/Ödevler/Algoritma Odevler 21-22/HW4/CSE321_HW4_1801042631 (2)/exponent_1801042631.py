def exponentiation_BF(number,base):
    result = 1
    for i in range(base):
        result*=number
    return result

def exponentiation_DAC(number,base):
    
    if (base == 0): 
        return 1
    result = exponentiation_DAC(number, int(base / 2))
    if (int(base % 2) == 0):
        
        return result * result
    else:
        return number * result * result

print(exponentiation_BF(4, 3))
print(exponentiation_DAC(4, 3))