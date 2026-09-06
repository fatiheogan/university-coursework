package Devices;

import Protocols.*;

public abstract class TempSensor extends Sensor {

    public TempSensor(String name, Protocol protocol) {
        super(name, protocol, "TempSensor");
    }

    public abstract float getTemp();

    public String data2String() {
        float temp = getTemp();
        return String.format("Temperature: %.2fC", temp);
    }
}
