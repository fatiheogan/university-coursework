def partition(arr, low, high):
	i = (low-1)
	pivot = arr[high]

	for j in range(low, high):

		if arr[j] <= pivot:

			i = i+1
			arr[i], arr[j] = arr[j], arr[i]

	arr[i+1], arr[high] = arr[high], arr[i+1]
	return (i+1)


def quickSort(arr, low, high):
	if len(arr) == 1:
		return arr
	if low < high:

		pi = partition(arr, low, high)

		quickSort(arr, low, pi-1)
		quickSort(arr, pi+1, high)

# driver
def main():
	boxes = [10, 7, 8, 9, 1, 5]
	gifts = [5, 7, 6, 2, 11, 8]

	print("BEFORE SORTING:")
	print("Boxes= ", boxes)
	print("Gifts= ", gifts)

	boxesLength = len(boxes)
	giftsLength = len(gifts)

	quickSort(boxes, 0, boxesLength-1)
	quickSort(gifts, 0, giftsLength-1)

	print()
	print("AFTER SORTING:")
	
	print("Boxes= ", boxes)
	print("Gifts= ", gifts)

	print()
	
	for i in range(0, boxesLength):
		if(boxes[i] < gifts[i]):
			print(i, "th Box is small for Gift #", i)
		elif(boxes[i] == gifts[i]):
			print(i, "th Box fits perfectly for Gift #", i)
		else:
			print(i, "th Box is bigger than Gift #", i)


main()