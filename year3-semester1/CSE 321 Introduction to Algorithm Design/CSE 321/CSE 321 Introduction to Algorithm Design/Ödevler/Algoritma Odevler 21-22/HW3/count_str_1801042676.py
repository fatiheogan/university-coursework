
def countSubstringBruteForce(str, start, end):
    count = 0
    for i in range(len(str)):
        if (str[i] == start):
            for j in range(i, len(str)):
                if(str[j] == end):
                    count = count +1
    return count    

def countSubstringBetter(str, start, end):
 
    totalSubstring = 0
    countStart = 0
 
    for i in range(len(str)):
 
        if str[i] == start:
            countStart = countStart + 1
        if str[i] == end:
            totalSubstring = totalSubstring + countStart
     
    return totalSubstring
 
def main():
	str = 'MuhammedSalihYılmaz'
	n = len(str)
	start = 'a'
	end = 'a'
	print('Substring count with Brute Force algorithm: ' , countSubstringBruteForce(str, start, end))
	print('Substring count with better algorithm: ' , countSubstringBetter(str, start, end))

main()