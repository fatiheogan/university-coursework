# name: Halil Ibrahim Oymaci
# no:	121044019
# date:	30.11.2015
# info:	Check whether an graph is a bipartite by using BFS (Bread Fist Search) algorithm
# references: Course book: Introduction to The Design and Analysis of Algorithms 3th edition: pages 123, 126, 129





# 1x - 2x
# | / |
# 4x - 3x
# This graph is bot bipartites and simulated above
notBipartiteGraph = [
	[0, 1, 0, 1],
	[1, 0, 1, 0],
	[0, 1, 0, 1],
	[1, 0, 1, 0]]


# x - x - x
# |   |   |
# x - x - x
# This graph is bipartites and simulated below
bipartiteGraph = [
	[0, 1, 0, 0, 0, 1],
    [1, 0, 1, 0, 1, 0],
    [0, 1, 0, 1, 0, 0],
    [0, 0, 1, 0, 1, 0],
    [0, 1, 0, 1, 0, 1],
    [1, 0, 0, 0, 1, 0]]



# check whether the given graph is a bipartite graph by using bread first search algorithm by starting 1. vertex
def isBipartiteByUsingBFS(graph):
	# colors of vertexes are initialized 0 which is not colored
	# 1 is red
	# 2 is black
	vertexColors = []
	for x in range(0,len(graph)):
		vertexColors.append(0)
	# if a vertex visited, initialized 1, otherwise 0
	vertexVisited = []
	for x in range(0,len(graph)):
		vertexVisited.append(0)

	# start deep-first-search from 1. vertex
	color = 1
	i = 0
	vertexColors[i] = color
	if ( color == 1 ):
		color = 2
	else:
		color = 1
	while( i < len(graph) ):
		j = 0
		while( j < len(graph[i]) ):
			if(graph[i][j]==1):
				vertexColors[j]=color
			j += 1
		if ( color == 1 ):
			color = 2
		else:
			color = 1
		i += 1
	#check if graph is bipartite
	i = 0
	while ( i < len(vertexColors)-1):
		if( vertexColors[i] == vertexColors[i+1] ):
			return False
		i += 1
	return True

print 'bipartite graph is bipartite: ' + str(isBipartiteByUsingBFS(bipartiteGraph))

print 'Not bipartite graph is bipartite: ' + str(isBipartiteByUsingBFS(notBipartiteGraph))
