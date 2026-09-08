from typing import List

"""
Standard Partition function of quicksort, with pivot element passed in.
"""
def Partition(arr, left, right, pivot):
    i = left
    j = left
    while j < right:
        if (arr[j] < pivot):
            arr[i], arr[j] = arr[j], arr[i]
            i = i+1
        elif (arr[j] == pivot):
            arr[j], arr[right] = arr[right], arr[j]
            j = j-1
        j = j+1
    arr[i], arr[right] = arr[right], arr[i]

    return i


"""
standard quicksort
:param arr1: gifts
:param arr2: boxes
:param left: left part of the array
:param right: right part of the array
"""
def SortGiftsByBoxes(arr1, arr2, left, right):
    if (left < right):

        pivot = Partition(arr1, left, right, arr2[right])

        Partition(arr2, left, right, arr1[pivot])

        SortGiftsByBoxes(arr1, arr2, left, pivot - 1)
        SortGiftsByBoxes(arr1, arr2, pivot + 1, right)


"""
printing function, for comparison
"""
def DoesFit(arr1, arr2, length):
    print("Fits?")
    for i in range(0, length):
        if(arr1[i] < arr2[i]):
            print(i, "th Box is small for Gift #", i)
        elif(arr1[i] == arr2[i]):
            print(i, "th Box fits perfectly for Gift #", i)
        else:
            print(i, "th Box is bigger than Gift #", i)


"""
main function
"""
def main():

    gifts = [2, 1, 3, 4, 6, 5, 8, 7]
    boxes = [1, 2, 3, 4, 5, 6, 7, 8]
    giftsLength = len(gifts)
    boxesLength = len(boxes)

    print("Before sorting:")
    print("Gifts:", gifts)
    print("Boxes:", boxes)
    print()
    DoesFit(boxes, gifts, boxesLength)

    print()

    print("Gifts, sorted like boxes ")
    SortGiftsByBoxes(gifts, boxes, 0, giftsLength-1)
    print("Gifts:", gifts)
    print("Boxes:", boxes)
    print()
    DoesFit(boxes, gifts, boxesLength)


main()