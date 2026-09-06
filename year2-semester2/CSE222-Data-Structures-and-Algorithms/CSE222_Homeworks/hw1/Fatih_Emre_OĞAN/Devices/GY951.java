package Devices;

import Protocols.Protocol;

public class GY951 extends IMUSensor {

    // GY951 cihazının constructorı
    public GY951(Protocol protocol) {
        super("GY951", protocol);  // Device constructor'ını çağırıyoruz
    }

    
    @Override
    public float getAccel() {
    
        float accel = (float) (Math.random() * 10); // 0 ile 10 arasında rastgele ivme değeri
        System.out.println("GY951: Reading acceleration: " + accel + " m/s²");
        return accel;
    }

    
    @Override
    public float getRot() {
       
        float rotation = (float) (Math.random() * 360); // 0 ile 360 derece arasında rastgele dönüş
        System.out.println("GY951: Reading rotation: " + rotation + " degrees");
        return rotation;
    }

    // Cihazın verilerini string formatında döndürme
    @Override
    public String data2String() {
        // Hem ivme hem de dönüş verilerini string olarak formatlıyoruz
        return String.format("Accel: %.2f, Rot: %.2f", getAccel(), getRot());
    }

    // Cihazın türünü döndürme (sensor türü: IMUSensor)
    @Override
    public String getSensType() {
        return "IMUSensor";
    }

    // Cihazın durumu
    @Override
    public State getState() {
        return state;
    }

    // Cihazı açma
    @Override
    public void turnON() {
        state = State.ON;
        System.out.println("GY951: Turning ON");
        protocol.write("turnON");
    }

    // Cihazı kapama
    @Override
    public void turnOFF() {
        state = State.OFF;
        System.out.println("GY951: Turning OFF");
        protocol.write("turnOFF");
    }
}
