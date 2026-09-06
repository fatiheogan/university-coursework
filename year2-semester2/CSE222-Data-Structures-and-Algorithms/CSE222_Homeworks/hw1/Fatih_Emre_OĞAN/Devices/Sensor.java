package Devices;

import Protocols.Protocol;

public abstract class Sensor extends Device {
    protected String sensType;

   
    public Sensor(String name, Protocol protocol, String sensType) {
        super(name, "Sensor");  // Device sınıfındaki constructor'ı çağırıyoruz, devType "Sensor"
        this.sensType = sensType;  // sensType'ı burada belirliyoruz
    }

    public abstract String data2String();

    public String getSensType() {
        return sensType;
    }

    @Override
    public String getDevType() {
        return this.devType;  // devType burada zaten "Sensor" olduğu için doğru
    }
}
