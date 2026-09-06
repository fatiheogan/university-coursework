/**
 * A task linked to a user and ID, used in the task scheduler.
 * Implements Comparable for sorting by user priority and task ID.
 */
public class MyTask implements Comparable<MyTask> {
    private MyUser user; // User who created the task
    private Integer id;   // Unique task ID

    /**
     * Creates a task with a user and ID.
     * Time complexity: O(1)
     * @param user the user who created the task
     * @param id the task's unique ID
     */
    public MyTask(MyUser user, Integer id) {
        this.user = user;
        this.id = id;
    }

    /**
     * Compares tasks by user priority (lower is more important) and then by task ID.
     * Time complexity: O(1)
     * @param other the task to compare with
     * @return negative, zero, or positive if this task is less, equal, or greater
     */
    @Override
    public int compareTo(MyTask other) {
        // Compare user priority first
        int priorityComparison = this.user.getPriority().compareTo(other.user.getPriority());
        if (priorityComparison != 0) {
            return priorityComparison;
        }
        // If priorities are equal, compare task ID
        return this.id.compareTo(other.id);
    }

    /**
     * Returns task as "Request {@code id} User {@code userID}".
     * Time complexity: O(1)
     * @return task as a string
     */
    @Override
    public String toString() {
        return "Task " + id + " User " + user.getID();
    }
}