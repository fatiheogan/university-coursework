# name: Halil Ibrahim Oymaci
# no:	121044019
# date:	30.11.2015
# info:	Sort an array decreasing according to brute force algorithm that is bubble sort

mixedArray = ['M','C','M','M','M','M','M','M','M','M','M']


def sortBruteForce( array ):
	i = 0
	while(i < len(array)-1):
		j = 0
		while(j < len(array)-1-i):
			if(array[j]<array[j+1]):
				temp = array[j]
				array[j]=array[j+1]
				array[j+1]=temp
			j+=1
		i+=1

print "before sorting"
print mixedArray
sortBruteForce(mixedArray)
print "after sorting"
print mixedArray

# Second test
# array2 = [6,9,3,2,8,0,12]
# print array2	
# sortBruteForce(array2)
# print array2