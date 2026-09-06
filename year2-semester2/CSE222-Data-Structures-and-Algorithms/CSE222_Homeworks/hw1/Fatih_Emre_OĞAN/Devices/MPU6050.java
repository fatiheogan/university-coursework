package Devices;

import Protocols.*;

public class MPU6050 extends IMUSensor {

    public MPU6050(Protocol protocol) {
        super("MPU6050", protocol);  // Cihaz adı ve protokolü burada belirtiyoruz.
    }

    @Override
    public float getAccel() {
        
        return (float) (Math.random() * 10);  // 0 ile 10 arasında rastgele ivme
    }

    @Override
    public float getRot() {
        // Rastgele dönüş hızı verisi döndürüyoruz. Gerçek cihazda bu değer sensörden alınır.
        return (float) (Math.random() * 100);  // 0 ile 100 arasında rastgele dönüş hızı
    }

    @Override
    public void turnON() {
        // MPU6050 cihazını açıyoruz ve protokole "turnON" yazıyoruz
        System.out.println(this.getName() + ": Turning ON");
        protocol.write("turnON");
    }

    @Override
    public void turnOFF() {
        
        System.out.println(this.getName() + ": Turning OFF");
        protocol.write("turnOFF");
    }

    @Override
    public State getState() {
        // MPU6050 cihazının durumunu döndürüyoruz
        return this.state;  
    }
}
