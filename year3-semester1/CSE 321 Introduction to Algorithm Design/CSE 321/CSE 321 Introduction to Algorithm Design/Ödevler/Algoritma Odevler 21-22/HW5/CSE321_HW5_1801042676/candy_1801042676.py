def CandyShop(capacity, lengths, prices):
	size = len(prices)
	#create an double array[][] filled with zeroes.
	array = [[0 for x in range(capacity + 1)] for x in range(size + 1)]

	for i in range(size + 1):
		for j in range(capacity + 1):
			if (i == 0 or j == 0):
				array[i][j] = 0
			elif lengths[i-1] <= j:
				array[i][j] = max(prices[i-1]
							  + array[i-1][j-lengths[i-1]],
							  array[i-1][j])
			else:
				array[i][j] = array[i-1][j]

	return array[size][capacity]


def main():
	prices = [1, 5, 8, 9, 10, 17, 17, 20]
	length = [1, 2, 3, 4, 5, 6, 7, 8]
	capacity = len(length)
	print(CandyShop(capacity, length, prices))

main()