import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;

/**
 * Manages tasks by scheduling them based on user priorities using a MinHeap.
 * Reads user priorities from a configuration file and accepts tasks via terminal input.
 */
public class TaskManager {
    private List<MyUser> users; // List to store users
    private MyPriorityQueue<MyTask> taskQueue; // MinHeap to store tasks
    private int nextTaskId; // Tracks the next task ID

    /**
     * Constructs a TaskManager by reading user priorities from the specified configuration file.
     * Time complexity: O(n) where n is the number of users
     * @param configFilePath the path to the configuration file
     */
    public TaskManager(String configFilePath) {
        this.users = new ArrayList<>();
        this.taskQueue = new MinHeap<>();
        this.nextTaskId = 0;
        readUsersFromFile(configFilePath);
    }

    /**
     * Reads user priorities from the configuration file and creates MyUser objects.
     * Time complexity: O(n) where n is the number of users
     * @param configFilePath the path to the configuration file
     */
    private void readUsersFromFile(String configFilePath) {
        try (BufferedReader reader = new BufferedReader(new FileReader(configFilePath))) {
            String line;
            int userId = 0;
            while ((line = reader.readLine()) != null) {
                try {
                    int priority = Integer.parseInt(line.trim());
                    users.add(new MyUser(userId, priority));
                    userId++;
                } catch (NumberFormatException e) {
                    System.err.println("Invalid priority in config file: " + line);
                }
            }
        } catch (IOException e) {
            System.err.println("Error reading config file: " + e.getMessage());
        }
    }

    /**
     * Runs the task manager, reading tasks from the terminal and executing them when "execute" is received.
     * Time complexity: O(m log m) where m is the number of tasks (due to heap operations)
     */
    public void run() {
        Scanner scanner = new Scanner(System.in);
        while (scanner.hasNextLine()) {
            String input = scanner.nextLine().trim();
            if (input.equals("execute")) {
                executeTasks();
                break;
            }
            try {
                int userId = Integer.parseInt(input);
                if (userId >= 0 && userId < users.size()) {
                    MyTask task = new MyTask(users.get(userId), nextTaskId++);
                    taskQueue.add(task);
                } else {
                    System.err.println("Invalid user ID: " + userId);
                }
            } catch (NumberFormatException e) {
                System.err.println("Invalid input: " + input);
            }
        }
        scanner.close();
    }

    /**
     * Executes all tasks in the queue by printing their toString output in priority order.
     * Time complexity: O(m log m) where m is the number of tasks
     */
    private void executeTasks() {
        while (!taskQueue.isEmpty()) {
            MyTask task = taskQueue.poll();
            if (task != null) {
                System.out.println(task.toString());
            }
        }
    }

    /**
     * Main method to start the TaskManager with a configuration file path.
     * @param args command line arguments, expects the config file path as the first argument
     */
    public static void main(String[] args) {
        if (args.length < 1) {
            System.err.println("Configuration file path is required.");
            return;
        }
        TaskManager manager = new TaskManager(args[0]);
        manager.run();
    }
}