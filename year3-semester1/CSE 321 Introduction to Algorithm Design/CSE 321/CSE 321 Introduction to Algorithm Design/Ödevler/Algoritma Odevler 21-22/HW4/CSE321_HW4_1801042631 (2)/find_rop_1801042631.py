def findRop(arr, n):
	temp_arr = [0]*n
	return _findRop(arr, temp_arr,0, n - 1)


def _findRop(arr, temp_arr, left, right):

	rop_count = 0

	if left < right:


		mid = (left + right)//2

		rop_count += _findRop(arr, temp_arr,
								left, mid)

		rop_count += _findRop(arr, temp_arr,
								mid + 1, right)

		rop_count += merge(arr, temp_arr,
						left, mid, right)
	return rop_count

def merge(arr, temp_arr, left, mid, right):


	i = left	
	j = mid + 1


	k = left	
	rop_count = 0


	while i <= mid and j <= right:

		if arr[i] <= arr[j]:
			temp_arr[k] = arr[i]
			k += 1
			i += 1
		else:

			temp_arr[k] = arr[j]
			rop_count += (mid-i + 1)
			k += 1
			j += 1

	while i <= mid:
		temp_arr[k] = arr[i]
		k += 1
		i += 1

	while j <= right:
		temp_arr[k] = arr[j]
		k += 1
		j += 1

	for loop_var in range(left, right + 1):
		arr[loop_var] = temp_arr[loop_var]
		
	return rop_count


receive_informations = [1, 20, 6, 4, 5]
n = len(receive_informations)
result = findRop(receive_informations, n)
print("The number of reverse-ordered pairs is ", result)
