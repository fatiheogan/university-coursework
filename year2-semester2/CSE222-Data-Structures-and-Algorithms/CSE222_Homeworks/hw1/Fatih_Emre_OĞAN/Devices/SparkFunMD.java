package Devices;

import Protocols.Protocol;

public class SparkFunMD extends MotorDriver {

   
    public SparkFunMD(Protocol protocol) {
        super("SparkFunMD", protocol); // Motor sürücüsü adı ve protocolü ile super constructor 
    }

    // Motor sürücüsünü açma işlemi
    @Override
    public void turnON() {
        System.out.println("SparkFunMD: Turning ON");
        protocol.write("turnON"); // Protocol üzerinden veriyi gönderme
        this.state = State.ON; // Durumu değiştirme
    }

    // Motor sürücüsünü kapama işlemi
    @Override
    public void turnOFF() {
        System.out.println("SparkFunMD: Turning OFF");
        protocol.write("turnOFF"); 
        this.state = State.OFF; // Durumu değiştirme
    }

    // Motor hızını ayarlama
    @Override
    public void setMotorSpeed(int speed) {
        System.out.println("SparkFunMD: Setting motor speed to " + speed);
        protocol.write("setSpeed " + speed); // Protocol üzerinden hız ayarını gönderme
    }

    // Motor sürücüsünün durumu döndürme
    @Override
    public State getState() {
        return this.state;
    }
}
