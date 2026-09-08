def mergeSort(arr, n):

    tempArr = [0]*n
    return ReverseOrderCount(arr, tempArr, 0, n-1)


def ReverseOrderCount(arr, tempArr, left, right):

    reverseCount = 0

    if left < right:

        #divide and floor it.
        mid = (left + right)//2

        reverseCount += ReverseOrderCount(arr, tempArr, left, mid)
        reverseCount += ReverseOrderCount(arr, tempArr, mid + 1, right)
        reverseCount += merge(arr, tempArr, left, mid, right)

    return reverseCount

#standard mergesort algorithm
def merge(arr, tempArr, left, mid, right):
    i = left
    j = mid + 1
    k = left
    reverseCount = 0

    while i <= mid and j <= right:

        if arr[i] <= arr[j]:
            tempArr[k] = arr[i]
            k += 1
            i += 1
        else:

            tempArr[k] = arr[j]
            reverseCount += (mid-i + 1)
            k += 1
            j += 1

    while i <= mid:
        tempArr[k] = arr[i]
        k += 1
        i += 1

    while j <= right:
        tempArr[k] = arr[j]
        k += 1
        j += 1

    for tempVar in range(left, right + 1):
        arr[tempVar] = tempArr[tempVar]

    return reverseCount


def main():
    arr = [98, 45, 2, 8, 35, 13, 32]
    n = len(arr)
    result = mergeSort(arr, n)
    print("Number of reverse-ordered pairs are", result)


main()