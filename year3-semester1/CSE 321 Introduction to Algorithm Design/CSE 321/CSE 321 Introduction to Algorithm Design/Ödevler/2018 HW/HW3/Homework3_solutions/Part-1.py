import pandas as pd
# The ReadFiletoGrap function takes
# the file name as a parameter and
# generates a Graph with the data in it.
def ReadFiletoGrap(fileName):
    graph = {}
    rootVertex = None
    data = pd.read_excel(fileName)
    rootVertex = data.columns.values.tolist()[1]
    dataLists = data.values
    i = 2
    while (i < len(dataLists)):
        keyVertex = dataLists[i][0]
        val = set()
        while (i < len(dataLists) and keyVertex == dataLists[i][0]):
            val.add(dataLists[i][1])
            i += 1
        graph[keyVertex] = val
    return graph, rootVertex

# It travels all vertexs within the Graph according to
# a Graph's root vertex with the DFS algorithm.
# This function returns the vertex list it travels.
def DFS(graph, startVertex, visited=[]):
    if startVertex not in visited:
        visited.append(startVertex)
    if startVertex in graph:
        for vertex in graph[startVertex] - set(visited):
            DFS(graph, vertex, visited)
    return visited

#it travels all vertexs within the Graph according to
#a Graph's root vertex with the BFS algorithm.
#This function returns the vertex list it travels.
def BFS(graph, startVertex, queue=[], visited=[]):
    if startVertex not in queue and startVertex not in visited:
        queue.append(startVertex)

    for item in list(graph[startVertex]):
        if item not in queue and item not in visited:
            queue.append(item)

    startVertex = queue[0]
    queue.remove(startVertex)
    visited.append(startVertex)

    if (startVertex in graph):
        BFS(graph, startVertex, queue, visited)

    if (len(queue) > 0):
        startVertex = queue[0]
        queue.remove(startVertex)
        visited.append(startVertex)

    return visited


def PrintList(string, tList):
    print(string, end='')
    for i in range(0, len(tList)):
        if (i < len(tList) - 1):
            print(str(tList[i]) + " -> ", end='')
        else:
            print(str(tList[i]))


def main():
    graph, rootVertex = ReadFiletoGrap('Graph_data.XLS')
    PrintList("BFS: ", BFS(graph, rootVertex))
    PrintList("DFS: ", DFS(graph, rootVertex))

# DFS Complexity Analysis
# Vertex => V Edge => E
# If your graph is implemented using adjacency lists,
# where each node maintains a list of all its adjacent edges,
# then, for each node, you could discover all its neighbors
# by traversing its adjacency list just once in linear time.
# T(n) = VO(1) + EO(1)
# T(n) = O(V) + O(E) ==> T(n) = O(V + E)
# However, In DFS, when we pass each node exactly once,
# the time complexity is at least T(n) = V*O(1) ==> T(n) = O(V).

# BFS Complexity Analysis
# The worst case (which is what O means),
# is basically for all vertices if the graph,
# do a O(1) operation with them - getting it from the queue,
# then for all outbound edges of v do another O(1) operation,
# adding their other end to the queue. Thus summing the first O(1)
# operation for all vertices gives O(V) and summing the O(outbound edge count)
# over all the vertices gives O(E), which gives O(V)+O(E)=O(V+E).
# Without the processing, or assuming it is O(1) like adding the vertex to a list.
# T(n) = O(V)+O(E) ==> T(n) = O(V+E)