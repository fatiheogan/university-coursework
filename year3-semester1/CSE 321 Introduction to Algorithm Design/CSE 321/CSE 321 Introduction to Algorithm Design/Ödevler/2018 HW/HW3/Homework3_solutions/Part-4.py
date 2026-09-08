def get_largest_sum_of_contiguous_subset(array):
    """This function finds a contiguous subset having the largest sum within given list."""
    if not array:  # Check whether list is empty or not. If it is empty return None.
        return None

    # The helper recursive function returns the lower and higher indexes of the subset.
    low, high = get_largest_sum_of_contiguous_subset_helper(array, 0, len(array) - 1)

    # Slices the list to get subset.
    return array[low:high + 1]


def get_largest_sum_of_contiguous_subset_helper(array, low, high):
    """Helper recursive function. Return the boundary of subset as low and high indexes."""

    if low == high:  # Check whether the list have one item.
        return (low, high)

    # Calculate the middle.
    mid = (low + high) // 2

    ################## DIVIDE PART ##################
    # Get the left largest sum of contiguous subset recursively.
    left_largest = get_largest_sum_of_contiguous_subset_helper(array, low, mid)

    # Get the right largest sum of contiguous subset recursively.
    right_largest = get_largest_sum_of_contiguous_subset_helper(array, mid + 1, high)

    ################## COMBINE PART ##################
    # Combine left and right subsets.
    return combine_left_and_right(array, left_largest, right_largest)


def combine_left_and_right(array, left, right):
    """Merges the left and right subsets."""

    # Calculate the sum of left part.
    left_sum = sum(array[left[0]:left[1] + 1])

    # Calculate the sum of right part.
    right_sum = sum(array[right[0]:right[1] + 1])

    # If left and right subsets are contiguous.
    if (right[0] - left[1]) == 1:

        # Calculate the both subset sum.
        left_right_sum = left_sum + right_sum

        # If it is greater or equal than right and left subset sum,
        # return low from left subset, high from right subset.
        if right_sum <= left_right_sum and left_sum <= left_right_sum:
            return (left[0], right[1])
    else:  # Not contiguous.

        # Calculate the sum of range.
        range_sum = sum(array[left[0]:right[1] + 1])

        # If range sum is greater than left and right subset sum, include items
        # between two subsets and return low and high indexes.
        if right_sum <= range_sum and left_sum <= range_sum:
            return (left[0], right[1])

    # Otherwise, return the subset that has largest sum.
    return right if left_sum < right_sum else left

test = [7, -6, 6, 7, -6, 7, -4, 3]
largest_sublist=get_largest_sum_of_contiguous_subset(test)
print("The largest contiguous subset for largest sum= ",largest_sublist,", Sum =",sum(largest_sublist))

# Algorithm Analysis
# The given list is divided into two part.
# Problem is solved for each part recursively.
# At the end, the results are combined. The divide part is straightforward.
# In combine part, the sum of left and right subsets are calculated separately
# using built-in sum function at first. If the two subsets are contiguous,
# then the sum of left and right subsets calculated.
# If the sum is greater than or equal to both subset sum,
# the two subsets are merged. If the subsets are not contiguous,
# the sum of elements are calculated from beginning of the left subset to end of the right subset.
# If the range sum is greater than or equal to both subset sum, the two subsets are merged.
# If these two cases are not satisfied, then the subset that has the largest sum returned.
# The return value of this function is tuple in which includes beginning and end of the subset.

# The complexity of dividing is O(logn) since the list is divided into two part.
# The complexity of merge operation is O(n) since the sum of both side is calculated and
# the total element number is n. (I assumed that the complexity of the built-in sum function is O(n)).
# Therefore, the complexity of this problem is O(nlogn).