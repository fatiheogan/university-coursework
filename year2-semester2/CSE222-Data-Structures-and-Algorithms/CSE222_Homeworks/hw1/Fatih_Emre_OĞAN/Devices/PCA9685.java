package Devices;

import Protocols.Protocol;

public class PCA9685 extends MotorDriver {

    
    private final int i2cAddress;

    // PCA9685 cihazının constructorı
    public PCA9685(Protocol protocol, int i2cAddress) {
        super("PCA9685", protocol);  // MotorDriver constructorını çağırıyoruz
        this.i2cAddress = i2cAddress;  // I2C adresini alıyoruz
        this.devType = "PCA9685";  // Cihaz türü olarak PCA9685'i belirtiyoruz
    }

    // Motor hızını ayarlama
    @Override
    public void setMotorSpeed(int speed) {
        
        
        // I2C protokolü kullanarak motor hızını ayarlıyoruz
        System.out.println("PCA9685: Setting motor speed to " + speed);
        
        // Protokole motor hızını ayarlama komutu gönderiyoruz
        protocol.writeToI2C(i2cAddress, "setMotorSpeed:" + speed);
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
        System.out.println("PCA9685: Turning ON");

        // Cihazı açmak için protokole komut gönderiyoruz
        protocol.writeToI2C(i2cAddress, "turnON");
    }

    // Cihazı kapama
    @Override
    public void turnOFF() {
        state = State.OFF;
        System.out.println("PCA9685: Turning OFF");

        // Cihazı kapatmak için protokole komut gönderiyoruz
        protocol.writeToI2C(i2cAddress, "turnOFF");
    }
}
