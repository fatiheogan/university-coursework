def CourseSelection(course, start, finish):
    n = len(start)
    array = [0]*n
    array[0] = course[0]

    i = 0
    count = 0

    for j in range(n):
        if(start[j] >= finish[i]):
            count = count+1
            array[count] = course[j]
            i = j
    return count, array


def main():

    course = ["English", "Mathematics", "Physics",
              "Chemistry", "Biology", "Geography"]
    start = [1, 3, 0, 5, 8, 5]
    finish = [2, 4, 6, 7, 9, 9]
    selectedCourseCount, selectedCourses = CourseSelection(
        course, start, finish)

    print(selectedCourseCount+1, "courses selected. Selected courses are:")
    for i in range(0, selectedCourseCount+1):
        print(selectedCourses[i])


main()
