package Devices;


import Protocols.*;

public abstract class IMUSensor extends Sensor {

    public IMUSensor(String name, Protocol protocol) {
        super(name, protocol, "IMUSensor");
    }

    public abstract float getAccel();

    public abstract float getRot();

    public String data2String() {
        float accel = getAccel();
        float rot = getRot();
        return String.format("Accel: %.2f, Rot: %.2f", accel, rot);
    }
}
