def MaximumProfit(arr):

    maximumSum = arr[0]
    currentSum = arr[0]

    for i in range(1, len(arr)):
        currentSum = currentSum + arr[i]
        print("1." , currentSum)
        currentSum = max(arr[i], currentSum)
        print("2." , currentSum)
        maximumSum = max(maximumSum, currentSum)
        print("3." , maximumSum)

    return maximumSum


def main():
    array = [-2, -3, 4, -1]
    print("The maximum profit that belongs to the most profitable cluster is:", MaximumProfit(array))

main()