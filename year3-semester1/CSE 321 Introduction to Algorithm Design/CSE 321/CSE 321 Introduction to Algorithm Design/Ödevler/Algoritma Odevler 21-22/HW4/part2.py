def findWorstAndBest(nums, left, right, min, max):

    # one element case
    if left == right:               
        if min > nums[right]:          
            min = nums[right]
        if max < nums[left]:           
            max = nums[left]
        return min, max

    #more than 1 element case
    mid = (left + right) // 2
    min, max = findWorstAndBest(nums, left, mid, min, max)
    min, max = findWorstAndBest(nums, mid + 1, right, min, max)

    return min, max


def main():

    arr = [98, 45, 2, 8, 35, 13, 32]

    minimum = float('inf')
    maximum = float('-inf')
    
    (min, max) = findWorstAndBest(arr, 0, len(arr) - 1, minimum, maximum)

    print("The worst outcome in the array is:", min)
    print("The best outcome in the array is", max)

main()