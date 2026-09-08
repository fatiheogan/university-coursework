def MaximumProfit(arr):

    maximumSum = arr[0]
    currentSum = arr[0]

    for i in range(1, len(arr)):
        currentSum = currentSum + arr[i]
        currentSum = max(arr[i], currentSum)
        maximumSum = max(maximumSum, currentSum)

    return maximumSum


def main():
    array = [3, -5, 2, 11, -8, 9, -5, ]
    print("The maximum profit that belongs to the most profitable cluster is:", MaximumProfit(array))

main()