/**
   * A user with an ID and priority.
   */
  public class MyUser {
    private Integer id;      // User ID
    private Integer priority; // User priority

    /**
     * Creates a user with given ID and priority.
     * Time complexity: O(1)
     * @param id the user ID
     * @param priority the user priority
     */
    public MyUser(Integer id, Integer priority) {
        this.id = id;
        this.priority = priority;
    }

    /**
     * Returns the user ID.
     * Time complexity: O(1)
     * @return the user ID
     */
    public Integer getID() {
        return id;
    }

    /**
     * Returns the user priority.
     * Time complexity: O(1)
     * @return the user priority
     */
    public Integer getPriority() {
        return priority;
    }
}