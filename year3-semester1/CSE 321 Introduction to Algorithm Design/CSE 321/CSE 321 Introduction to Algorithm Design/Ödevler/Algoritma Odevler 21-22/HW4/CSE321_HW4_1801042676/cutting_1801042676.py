def cut(nMeter, count):
    if(nMeter > 1):
        count += 1
        return (cut(nMeter/2, count))
    else:
        return count


def main():
	length = 8
	numberOfMinimumCuts = cut(8, 0)
	print("Number of minimum cuts needed for " + str(length) + " meters is: " + str(numberOfMinimumCuts))
	
main()