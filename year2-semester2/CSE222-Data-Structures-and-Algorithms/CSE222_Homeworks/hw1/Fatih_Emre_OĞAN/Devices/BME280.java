package Devices;

import Protocols.Protocol;

public class BME280 extends TempSensor {

    // BME280 cihazının constructorı
    public BME280(Protocol protocol) {
        super("BME280", protocol);  // Device constructor'ını çağırıyoruz
    }

    // BME280 cihazı için sıcaklık verisi alma
    @Override
    public float getTemp() {
    
        float temperature = (float) (Math.random() * 40); // 0 ile 40 derece arasında rastgele sıcaklık
        System.out.println("BME280: Reading temperature: " + temperature + "C");
        return temperature;
    }

    // Cihazın verilerini string formatında döndürme
    @Override
    public String data2String() {

        return String.format("Temperature: %.2fC", getTemp());
    }

    @Override
    public String getSensType() {
        return "TempSensor";
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
        System.out.println("BME280: Turning ON");
        protocol.write("turnON");
    }

    // Cihazı kapama
    @Override
    public void turnOFF() {
        state = State.OFF;
        System.out.println("BME280: Turning OFF");
        protocol.write("turnOFF");
    }
}
