def findFixedPoint(arr, leftValue, rightValue):
    midlePoint = None
    if rightValue >= leftValue:
        midlePoint = (leftValue + rightValue)//2
    if midlePoint is not None:
        if midlePoint is arr[midlePoint]:
            return int(midlePoint)

        if midlePoint > arr[midlePoint]:
            return findFixedPoint(arr, (midlePoint + 1), rightValue)
        else:
            return findFixedPoint(arr, leftValue, (midlePoint -1))
    else:
        return "There is not Fixed Point"

if __name__ == "__main__":
    arr = [-10, 0, 1, 3, 30, 50, 110]
    result = findFixedPoint(sorted(arr), 0, len(arr) - 1)
    if (isinstance(result, str)):
        print(result)
    else:
        print("Fixed Point is " + str(result))

# This function, which determines whether
# the index value is the same as the index value in the list,
# applies the “Divide and Conquer” algorithm.
# Each time the function is called, the array is divided by two.
# Therefore, if the middle point value of this function
# is the same as the index value in the first two divisions,
# then “Best - Case” is T(n) = O(1) Constant Time.
# However, if the middle point value is not the same as the index value
# in the two segments, the “Worst - Case” T(n) = T(n/2) + c
# “Assume that, Backward Substitution Method or Forward Substitution Method is applied”
# As a result, T(n) = O(log n) is Logarithmic Time.

