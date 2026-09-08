import sys


def giveConnectedComponentsBFS(graph, start, checked):
    grafQueue = [start]
    while grafQueue:
        vertex = grafQueue.pop(0)
        if vertex not in checked:
            components = graph[vertex]
            checked.append(vertex)
            for component in components:
                grafQueue.append(component)

def bubbleSort(lst):
    for member in range(len(lst)-1,0,-1):
        for i in range(member):
            if (lst[i]>lst[i+1]):
                temp = lst[i]
                lst[i] = lst[i+1]
                lst[i+1] = temp



def findDifferenConnectedComponents(graf):
  
  tempList = []
  
  for key in graf:
    temp = []
    giveConnectedComponentsBFS(graf,key,temp)
    bubbleSort(temp)
    if not temp in tempList:
      tempList.append(temp)
    
  return tempList

def findMinimumCostToLabifyGTU(costOfLab,costOfWay,graph):
  uniqueList = findDifferenConnectedComponents(graph)
  
  sumOfOnlyLabBuild = 0
  sumOfLabAndWay = 0

  for lst in uniqueList:
    sumOfOnlyLabBuild = sumOfOnlyLabBuild + len(lst) * costOfLab
  
  sumOfLabAndWay = costOfLab * len(uniqueList)
  
  for lst in uniqueList:
    sumOfLabAndWay = sumOfLabAndWay + (costOfWay * (len(lst)-1))
  
  if(sumOfLabAndWay>sumOfOnlyLabBuild):
    return sumOfOnlyLabBuild
  else:
    return sumOfLabAndWay

  
  

mapOfGTU = {  1 : set([2,3]),     
              2 : set([1,3,4]),     
              3 : set([1,2,4]),     
              4 : set([3,2]),     
              5 : set([6]),     
              6 : set([5]) }


minCost = findMinimumCostToLabifyGTU(5,2,mapOfGTU)
print(minCost)



