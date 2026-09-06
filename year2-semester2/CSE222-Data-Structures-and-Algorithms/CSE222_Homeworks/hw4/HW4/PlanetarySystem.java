import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;
import java.util.Stack;

/**
 * Represents a planetary system organized as a tree structure, with a star as the root node
 * and planets and satellites as child nodes. Provides methods to manage celestial bodies,
 * analyze sensor data, and generate mission reports.
 * 
 * @author [Your Name or Team Name, if required]
 */
public class PlanetarySystem {

    /** The root node of the planetary system, representing the main star. */
    private Node root;

    /**
     * Creates a new planetary system with a star as the root node.
     * 
     * @param starName    the name of the star
     * @param temperature the temperature of the star (in Kelvin)
     * @param pressure    the pressure of the star (in Pascals)
     * @param humidity    the humidity level (must be zero for stars)
     * @param radiation   the radiation level (in Sieverts)
     * @throws IllegalStateException  if a planetary system already exists
     * @throws IllegalArgumentException if humidity is not zero
     */
    public void createPlanetSystem(String starName, double temperature, double pressure, 
                                   double humidity, double radiation) {
        if (root != null) {
            throw new IllegalStateException("A planetary system already exists!");
        }
        if (humidity != 0) {
            throw new IllegalArgumentException("Stars cannot have humidity!");
        }
        SensorData data = new SensorData(temperature, pressure, humidity, radiation);
        root = new Node(starName, "Star", data);
        System.out.println("Planetary system created with star: " + starName);
    }

    /**
     * Adds a planet to the planetary system under a specified parent node.
     * 
     * @param planetName  the name of the planet
     * @param parentName  the name of the parent node (star or planet)
     * @param temperature the temperature of the planet (in Kelvin)
     * @param pressure    the pressure of the planet (in Pascals)
     * @param humidity    the humidity level of the planet
     * @param radiation   the radiation level (in Sieverts)
     */
    public void addPlanet(String planetName, String parentName, double temperature, 
                          double pressure, double humidity, double radiation) {
        if (temperature < 0 || pressure < 0 || humidity < 0 || radiation < 0) {
            System.out.println("Invalid sensor data: Values cannot be negative!");
            return;
        }
        if (findNode(root, planetName) != null) {
            System.out.println("A node with name " + planetName + " already exists!");
            return;
        }
        SensorData data = new SensorData(temperature, pressure, humidity, radiation);
        Node planet = new Node(planetName, "Planet", data);
        Node parent = findNode(root, parentName);
        if (parent == null) {
            System.out.println("Parent " + parentName + " not found!");
        } else if (parent.getType().equals("Moon")) {
            System.out.println("Cannot add a planet to a moon!");
        } else {
            parent.addChild(planet);
            System.out.println("Planet " + planetName + " added under " + parentName);
        }
    }

    /**
     * Adds a satellite (moon) to a planet in the planetary system.
     * 
     * @param satelliteName the name of the satellite
     * @param parentName    the name of the parent planet
     * @param temperature   the temperature of the satellite (in Kelvin)
     * @param pressure      the pressure of the satellite (in Pascals)
     * @param humidity      the humidity level of the satellite
     * @param radiation     the radiation level (in Sieverts)
     */
    public void addSatellite(String satelliteName, String parentName, double temperature, 
                             double pressure, double humidity, double radiation) {
        if (temperature < 0 || pressure < 0 || humidity < 0 || radiation < 0) {
            System.out.println("Invalid sensor data: Values cannot be negative!");
            return;
        }
        if (findNode(root, satelliteName) != null) {
            System.out.println("A node with name " + satelliteName + " already exists!");
            return;
        }
        SensorData data = new SensorData(temperature, pressure, humidity, radiation);
        Node satellite = new Node(satelliteName, "Moon", data);
        Node parent = findNode(root, parentName);
        if (parent != null && parent.getType().equals("Planet")) {
            parent.addChild(satellite);
            System.out.println("Satellite " + satelliteName + " added under " + parentName);
        } else {
            System.out.println("Parent " + parentName + " not found or not a planet!");
        }
    }

    /**
     * Finds all nodes in the planetary system with radiation levels above a specified threshold.
     * 
     * @param threshold the radiation threshold (in Sieverts)
     * @return a list of nodes with radiation above the threshold
     */
    public List<Node> findRadiationAnomalies(double threshold) {
        List<Node> anomalies = new ArrayList<>();
        if (root != null) {
            findRadiationAnomaliesRecursive(root, threshold, anomalies);
        }
        return anomalies;
    }

    /**
     * Recursively traverses the tree to find nodes with radiation above the threshold.
     * 
     * @param current   the current node being processed
     * @param threshold the radiation threshold (in Sieverts)
     * @param anomalies the list to store nodes with high radiation
     */
    private void findRadiationAnomaliesRecursive(Node current, double threshold, List<Node> anomalies) {
        if (current.getSensorData().getRadiation() > threshold) {
            anomalies.add(current);
        }
        for (Node child : current.getChildren()) {
            findRadiationAnomaliesRecursive(child, threshold, anomalies);
        }
    }

    /**
     * Returns the path from the root star to a specified node.
     * 
     * @param planetName the name of the target node
     * @return a stack containing the names of nodes in the path, or an empty stack if not found
     */
    public Stack<String> getPathTo(String planetName) {
        Stack<String> path = new Stack<>();
        if (root != null && findPathRecursive(root, planetName, path)) {
            return path;
        }
        return new Stack<>();
    }

    /**
     * Recursively searches for a node and builds the path to it.
     * 
     * @param current    the current node being processed
     * @param targetName the name of the target node
     * @param path       the stack to store the path
     * @return true if the target node is found, false otherwise
     */
    private boolean findPathRecursive(Node current, String targetName, Stack<String> path) {
        path.push(current.getName());
        if (current.getName().equals(targetName)) {
            return true;
        }
        for (Node child : current.getChildren()) {
            if (findPathRecursive(child, targetName, path)) {
                return true;
            }
        }
        path.pop();
        return false;
    }

    /**
     * Prints a mission report for the entire planetary system.
     */
    public void printMissionReport() {
        if (root == null) {
            System.out.println("No planetary system exists!");
            return;
        }
        printTreeRecursive(root, 0);
    }

    /**
     * Prints a mission report for a specific node.
     * 
     * @param nodeName the name of the node
     */
    public void printMissionReport(String nodeName) {
        Node node = findNode(root, nodeName);
        if (node != null) {
            System.out.println(node.getType() + " " + node.getName() + ": " + node.getSensorData());
        } else {
            System.out.println("Node " + nodeName + " not found!");
        }
    }

    /**
     * Recursively prints the tree structure with indentation based on depth.
     * 
     * @param current the current node being processed
     * @param depth   the depth of the current node in the tree
     */
    private void printTreeRecursive(Node current, int depth) {
        String indent = "  ".repeat(depth);
        System.out.println(indent + current.getType() + " " + current.getName() + ": " + current.getSensorData());
        for (Node child : current.getChildren()) {
            printTreeRecursive(child, depth + 1);
        }
    }

    /**
     * Searches for a node by name in the planetary system.
     * 
     * @param current the current node being processed
     * @param name    the name of the target node
     * @return the node if found, or null if not found
     */
    private Node findNode(Node current, String name) {
        if (current == null) return null;
        if (current.getName().equals(name)) return current;
        for (Node child : current.getChildren()) {
            Node found = findNode(child, name);
            if (found != null) return found;
        }
        return null;
    }

    /**
     * Executes a command based on the provided input parts.
     * 
     * @param parts the array of command arguments
     * @throws NumberFormatException      if numerical arguments are invalid
     * @throws ArrayIndexOutOfBoundsException if insufficient arguments are provided
     * @throws IllegalArgumentException   if command arguments are invalid
     * @throws IllegalStateException      if the system state does not allow the command
     */
    private void executeCommand(String[] parts) {
        try {
            switch (parts[0]) {
                case "create":
                    if (parts[1].equals("planetSystem")) {
                        createPlanetSystem(parts[2], Double.parseDouble(parts[3]), 
                            Double.parseDouble(parts[4]), Double.parseDouble(parts[5]), 
                            Double.parseDouble(parts[6]));
                    } else {
                        System.out.println("Invalid create command. Use: create planetSystem");
                    }
                    break;
                case "addPlanet":
                    addPlanet(parts[1], parts[2], Double.parseDouble(parts[3]), 
                        Double.parseDouble(parts[4]), Double.parseDouble(parts[5]), 
                        Double.parseDouble(parts[6]));
                    break;
                case "addSatellite":
                    addSatellite(parts[1], parts[2], Double.parseDouble(parts[3]), 
                        Double.parseDouble(parts[4]), Double.parseDouble(parts[5]), 
                        Double.parseDouble(parts[6]));
                    break;
                case "findRadiationAnomalies":
                    List<Node> anomalies = findRadiationAnomalies(Double.parseDouble(parts[1]));
                    System.out.println("Radiation anomalies (>" + parts[1] + " Sv):");
                    for (Node node : anomalies) {
                        System.out.println(node.getType() + " " + node.getName() + ": " + 
                            node.getSensorData().getRadiation());
                    }
                    break;
                case "getPathTo":
                    Stack<String> path = getPathTo(parts[1]);
                    System.out.println("Path to " + parts[1] + ": " + path);
                    break;
                case "printMissionReport":
                    if (parts.length == 1) {
                        printMissionReport();
                    } else {
                        printMissionReport(parts[1]);
                    }
                    break;
                case "exit":
                    System.out.println("Exiting program...");
                    System.exit(0);
                    break;
                default:
                    System.out.println("Unknown command: " + parts[0]);
            }
        } catch (NumberFormatException e) {
            System.out.println("Invalid number format in command: " + String.join(" ", parts));
        } catch (ArrayIndexOutOfBoundsException e) {
            System.out.println("Insufficient arguments for command: " + String.join(" ", parts));
        } catch (IllegalArgumentException | IllegalStateException e) {
            System.out.println(e.getMessage());
        }
    }

    /**
     * Main entry point for the planetary system terminal interface.
     * Reads commands from standard input and processes them.
     * 
     * @param args command-line arguments (not used)
     */
    public static void main(String[] args) {
        PlanetarySystem system = new PlanetarySystem();
        Scanner scanner = new Scanner(System.in);
        System.out.println("Planetary System Terminal UI.");
        System.out.println("Paste your commands below (end with an empty line or 'exit' to finish):");
        System.out.println("Available commands: create planetSystem, addPlanet, addSatellite, " +
                           "findRadiationAnomalies, getPathTo, printMissionReport");

        StringBuilder inputBuilder = new StringBuilder();
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine().trim();
            if (line.isEmpty() || line.equals("exit")) {
                break;
            }
            inputBuilder.append(line).append("\n");
        }

        String[] lines = inputBuilder.toString().split("\n");
        for (String line : lines) {
            if (!line.trim().isEmpty()) {
                System.out.println("\nExecuting: " + line);
                String[] parts = line.split("\\s+");
                system.executeCommand(parts);
            }
        }
        System.out.println("Processing complete.");
        scanner.close();
    }
}