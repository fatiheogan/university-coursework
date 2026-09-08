import sys
inputTable = [[5, 8, 7],  [8, 12, 7],  [4, 8,  5]] 

def permutasyon(product,size,mylist):
  if (size == 1 ):
    mylist.append(list(product))
    return product
    
  for i in range(0,size):
    permutasyon(product,size-1,mylist)
    if(size%2 == 1):
      temp = product[0]
      product[0] = product[size-1]
      product[size-1] = temp

    else:
      temp = product[i]
      product[i] = product[size-1]
      product[size-1] = temp


def editListByRule(list,numOfClass):
  for i in range(0,len(list)):
    for j in range(0,len(list[i])):
      if (list[i][j] > numOfClass-1):
          list[i][j] = -1

def findOptimalAssistantship(inputTable):

  
  for i in range(0,len(inputTable)):  
    if (len(inputTable[i]) > len(inputTable)):
      return []
  
  
  product =  [None] * len(inputTable)
  
  for i in range(0,len(product)):
    product[i] = i
  
  
  opportunities = []

 
  permutasyon(product,len(product),opportunities)
  editListByRule(opportunities,len(inputTable[0]))
  print(opportunities)
  
  timeList =  [None] * len(opportunities)
  for i in range(0,len(opportunities)):
    timeList[i] = 0
  
  
  for i in range(0, len(opportunities)):
    for j in range(0,len(inputTable)):
      if(not opportunities[i][j] == -1):
        timeList[i] = timeList[i] + inputTable[j][opportunities[i][j]]
      else :
        timeList[i] = timeList[i] + 6

  min = timeList[1]
  index = 0
  for i in range(1,len(timeList)):
    if(timeList[i]<min):
      min = timeList[i]
      index = i
     
  return opportunities[index], min
  
  

asst, time = findOptimalAssistantship(inputTable) 
print(asst) 
print(time)
  
    


