def partition(arr, l, r):
     
    x = arr[r]
    i = l
    for j in range(l, r):
         
        if arr[j] <= x:
            arr[i], arr[j] = arr[j], arr[i]
            i += 1
             
    arr[i], arr[r] = arr[r], arr[i]
    return i
 
def meaningful(arr, l, r, k):
 

    if (k > 0 and k <= r - l + 1):
 
        index = partition(arr, l, r)

        if (index - l == k - 1):
            return arr[index]
 
        if (index - l > k - 1):
            return meaningful(arr, l, index - 1, k)

        return meaningful(arr, index + 1, r,
                            k - index + l - 1)
    
experiment_results = [ 10, 7, 8, 9, 1, 5 ]
n = len(experiment_results)
k = 3
print("The success rate of the first meaningful {}th experiment is {}".format(k,meaningful(experiment_results,0,n-1,k)))