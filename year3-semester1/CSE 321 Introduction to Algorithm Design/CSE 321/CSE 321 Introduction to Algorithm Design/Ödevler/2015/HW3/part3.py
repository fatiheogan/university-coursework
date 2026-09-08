# name: Halil Ibrahim Oymaci
# no:	121044019
# date:	30.11.2015
# info:	Find the minimum PDV (positional difference value) in a list

# test lists
list = ['pazazz','jazzed','zizzle','muzjik','whizzy','scuzzy','fuzzed','puzzle']
list2 = ['qwerty','asdfgh','zxcvbn','tyughj','ghjbnm','ugojkl','laiopq','pzxjil']
list3 = ['davet','yavuz','havuz']

# find PDV in given two string with equal length
def findPDV(string1, string2):
	count = 0
	i = 0
	while(i < len(string1)):
		if(string1[i]!=string2[i]):
			count += 1
		i += 1
	return count

# find minimum PDV in a list
def findMinPDV(list):
	min = findPDV(list[0],list[1])
	closestStringIndex1 = 0
	closestStringIndex2 = 1
	i = 0
	while(i < len(list)):
		j = 0
		while(j < len(list)):
			if(i!=j):
				res = findPDV(list[i],list[j])
				if(res<min):
					min = res
					closestStringIndex1 = i
					closestStringIndex2 = j
			j += 1
		i += 1
	print list[closestStringIndex1]
	print list[closestStringIndex2]
	return min

print findMinPDV(list3)
