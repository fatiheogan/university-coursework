/**
 * Stores sensor data for a celestial body in the planetary system.
 * Includes measurements for temperature, pressure, humidity, and radiation.
 */
public class SensorData {
    // Temperature in Kelvin
    private double temperature;
    // Pressure in Pascals
    private double pressure;
    // Humidity as a percentage (0-100)
    private double humidity;
    // Radiation in Sieverts
    private double radiation;

    /**
     * Constructs a new SensorData object with the specified values.
     *
     * @param temperature the temperature in Kelvin
     * @param pressure    the pressure in Pascals
     * @param humidity    the humidity percentage (0-100)
     * @param radiation   the radiation in Sieverts
     */
    public SensorData(double temperature, double pressure, double humidity, double radiation) {
        this.temperature = temperature;
        this.pressure = pressure;
        this.humidity = humidity;
        this.radiation = radiation;
    }

    /**
     * Gets the temperature value.
     *
     * @return the temperature in Kelvin
     */
    public double getTemperature() { return temperature; }

    /**
     * Gets the pressure value.
     *
     * @return the pressure in Pascals
     */
    public double getPressure() { return pressure; }

    /**
     * Gets the humidity value.
     *
     * @return the humidity percentage
     */
    public double getHumidity() { return humidity; }

    /**
     * Gets the radiation value.
     *
     * @return the radiation in Sieverts
     */
    public double getRadiation() { return radiation; }

    /**
     * Returns a string representation of the sensor data.
     *
     * @return a formatted string containing temperature, pressure, humidity, and radiation
     */
    @Override
    public String toString() {
        return String.format("%s Kelvin, %s Pascals, %s%% Humidity, %s Sieverts",
                temperature, pressure, humidity, radiation);
    }
}