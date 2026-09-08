# searches the element - binary search
def first(arr, low, high, x, num):
	if (high >= low):
		mid = low + (high - low) // 2  # (low + high)/2
		if ((mid == 0 or x > arr[mid-1]) and arr[mid] == x):
			return mid
		if (x > arr[mid]):
			return first(arr, (mid + 1), high, x, num)
		return first(arr, low, (mid - 1), x, num)

	return -1


def sortGiftsByBoxes(A1, A2, A1Length, A2Length):

	temp = [0] * A1Length
	visitedElements = [0] * A1Length

	for i in range(0, A1Length):
		temp[i] = A1[i]
		visitedElements[i] = 0

	temp.sort() #uses Timsort. An upgraded way of quicksort

	index = 0

	for i in range(A2Length):

		f = first(temp, 0, A1Length-1, A2[i], A1Length)
		if (f == -1):
			continue
		j = f
		while (j < A1Length and temp[j] == A2[i]):
			A1[index] = temp[j]
			index = index + 1
			visitedElements[j] = 1
			j = j + 1

	for i in range(A1Length):
		if (visitedElements[i] == 0):
			A1[index] = temp[i]
			index = index + 1

def doesFit(A1, A2, length):
	for i in range(0, length):
		if(A1[i] < A2[i]):
			print(i, "th Box is small for Gift #", i)
		elif(A1[i] == A2[i]):
			print(i, "th Box fits perfectly for Gift #", i)
		else:
			print(i, "th Box is bigger than Gift #", i)

def main():
	gifts = [2, 1, 3, 4, 6, 5]
	boxes = [1, 2, 3, 4, 5, 6]
	giftsLength = len(gifts)
	boxesLength = len(boxes)

	print("Before sorting:")
	print ("Gifts:", gifts)
	print ("Boxes:", boxes)
	print()
	print("Fits?")
	doesFit(boxes, gifts, boxesLength)

	print()

	print("Gifts, sorted like boxes ")
	sortGiftsByBoxes(gifts, boxes, giftsLength, boxesLength)
	print ("Gifts:", gifts)
	print ("Boxes:", boxes)
	print()
	print("Fits?")
	doesFit(boxes, gifts, boxesLength)

main()
