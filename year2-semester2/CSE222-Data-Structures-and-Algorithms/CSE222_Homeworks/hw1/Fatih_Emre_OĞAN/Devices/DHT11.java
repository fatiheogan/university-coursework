package Devices;

import Protocols.*;

public class DHT11 extends TempSensor {

    public DHT11(Protocol protocol) {
        super("DHT11", protocol);  // Cihaz adı ve protokolü burada belirtiyoruz
    }

    @Override
    public float getTemp() {
        // Rastgele sıcaklık verisi döndürüyoruz. Gerçek cihazda bu değer sensörden alınır.
        return 25 + (float) Math.random() * 5;  // 25 ile 30 arasında rastgele bir sıcaklık
    }

    @Override
    public void turnON() {
        // DHT11 cihazını açıyoruz ve protokole "turnON" yazıyoruz
        if (protocol != null) {  // Protokol null kontrolü ekliyoruz
            System.out.println(this.getName() + ": Turning ON");
            protocol.write("turnON");
        } else {
            System.out.println("Protocol is null. Cannot turn on device.");
        }
    }

    @Override
    public void turnOFF() {
        // DHT11 cihazını kapatıyoruz ve protokole "turnOFF" yazıyoruz
        if (protocol != null) {  // Protokol null kontrolü ekliyoruz
            System.out.println(this.getName() + ": Turning OFF");
            protocol.write("turnOFF");
        } else {
            System.out.println("Protocol is null. Cannot turn off device.");
        }
    }

    @Override
    public State getState() {
    
        return this.state;  
    }
}
