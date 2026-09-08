def cutting(num:float,count:int):
    if(num > 1):
        count+=1
        return cutting(num/2, count)
    else:
        return count

number = 17
print("The minimum number of cuts for " +str(number) + " is "+ str(cutting(17, 0)))


