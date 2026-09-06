package Devices;

import Protocols.Protocol;

public abstract class MotorDriver extends Device {

    // MotorDriver sınıfının constructor'ı
    public MotorDriver(String name, Protocol protocol) {
        super(name, "MotorDriver");  // MotorDriver manuel olarak veriyoruz
        this.protocol = protocol;  // Protocol'ü burada atıyoruz
    }
    

    // Motor hızını ayarlamak için abstract metot
    public abstract void setMotorSpeed(int speed);
}
