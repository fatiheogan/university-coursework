def sum(values, start, end):
	result = 0
	for i in range(start, end):
		result = result + values[i]
	return result

# brute force
def ProfitableCluster(regions, profits):
	maxProfit = profits[0]
	cluster = [regions[0]]
	for i in range(len(regions)):
		for j in range(i, len(regions)):
			p = sum(profits, i, j)
			if (maxProfit < p):
				maxProfit = p
				cluster = regions[i:j]
	# print(maxProfit)
	return cluster


def DivideAndConquerSum(arr, left, middle, right):

	sum = 0
	left_sum = float('-inf')

	for i in range(middle, left-1, -1):
		sum = sum + arr[i]

		if (sum > left_sum):
			left_sum = sum

	sum = 0
	right_sum = float('-inf')
	for i in range(middle + 1, right + 1):
		sum = sum + arr[i]

		if (sum > right_sum):
			right_sum = sum

	return max(left_sum + right_sum,
				left_sum,
				right_sum)

# Divide and Conquer
def MaximumProfit(profit, left, right):

	if (left == right):
		return profit[left]

	middle = (left + right) // 2

	return max(MaximumProfit(profit, left, middle),
			   MaximumProfit(profit, middle+1, right),
			   DivideAndConquerSum(profit, left, middle, right))


def main():
	regions = ["A", "B", "C", "D", "E", "F", "G"]
	profits = [3, -5, 2, 11, -8, 9, -5]

	n = len(profits)

	print(ProfitableCluster(regions, profits))

	print("Maximum profit is", MaximumProfit(profits, 0, n-1))


main()
