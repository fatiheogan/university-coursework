def kthMeaningful(arr, left, right, k):

    if (k > 0 and k <= right - left + 1):
        position = partition(arr, left, right)

        if (position - left == k - 1):
            return arr[position]
        if (position - left > k - 1):
            return kthMeaningful(arr, left, position - 1, k)

        return kthMeaningful(arr, position + 1, right, k - position + left - 1)

    return float('inf')


# Partition of QuickSort() to implement Quick Select algorithm.
def partition(arr, l, r):

    x = arr[r]
    i = l
    for j in range(l, r):
        if (arr[j] <= x):
            arr[i], arr[j] = arr[j], arr[i]
            i += 1
    arr[i], arr[r] = arr[r], arr[i]
    return i


def main():

    arr = [98, 45, 2, 8, 35, 13, 32]
    n = len(arr)
    k = 3
    print("Kth meaningful experiment is", kthMeaningful(arr, 0, n - 1, k))


main()
