import java.util.ArrayList;
import java.util.List;

/**
 * Represents a celestial body in a planetary system, such as a star, planet, or moon.
 * Each node contains a name, type, sensor data, and a list of child nodes (e.g., planets for a star, moons for a planet).
 * 
 * @author [Fatih Emre OĞAN] 
 * @version 1.0
 */
public class Node {
    /**
     * The name of the celestial body (e.g., "Sun", "Earth", "Moon").
     */
    private String name;

    /**
     * The type of the celestial body. Valid types are "Star", "Planet", or "Moon".
     */
    private String type;

    /**
     * The sensor data associated with this celestial body.
     */
    private SensorData sensorData;

    /**
     * The list of child nodes representing orbiting bodies (e.g., planets for a star, moons for a planet).
     */
    private List<Node> children;

    /**
     * Constructs a new {@code Node} with the specified name, type, and sensor data.
     * Initializes an empty list for child nodes.
     * 
     * @param name       the name of the celestial body
     * @param type       the type of the celestial body ("Star", "Planet", or "Moon")
     * @param sensorData the sensor data associated with the body
     */
    public Node(String name, String type, SensorData sensorData) {
        this.name = name;
        this.type = type;
        this.sensorData = sensorData;
        this.children = new ArrayList<>();
    }

    /**
     * Returns the name of the celestial body.
     * 
     * @return the name of the node
     */
    public String getName() {
        return name;
    }

    /**
     * Returns the type of the celestial body.
     * 
     * @return the type of the node ("Star", "Planet", or "Moon")
     */
    public String getType() {
        return type;
    }

    /**
     * Returns the sensor data associated with the celestial body.
     * 
     * @return the sensor data
     */
    public SensorData getSensorData() {
        return sensorData;
    }

    /**
     * Returns the list of child nodes (e.g., planets or moons orbiting this body).
     * 
     * @return the list of child nodes
     */
    public List<Node> getChildren() {
        return children;
    }

    /**
     * Adds a child node to this celestial body’s list of children.
     * For example, adds a moon to a planet or a planet to a star.
     * 
     * @param child the child node to be added
     */
    public void addChild(Node child) {
        children.add(child);
    }
}