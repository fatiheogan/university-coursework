package Devices;

import Protocols.Protocol;

public class Bluetooth extends WirelessIO {

    // Bluetooth cihazının constructorı
    public Bluetooth(Protocol protocol) {
        super("Bluetooth", protocol); 
    }

    // Bluetooth için veri gönderme işlemi
    @Override
    public void sendData(String data) {
        System.out.println("Bluetooth: Sending \"" + data + "\".");
        protocol.write(data);  
    }

    // Bluetooth için veri alma işlemi
    @Override
    public String recvData() {
        String data = protocol.read();  
        System.out.println("Bluetooth: Receiving data.");
        return data;
    }

    // Bluetooth cihazının durumu
    @Override
    public State getState() {
        return state;  
    }

    // Bluetooth cihazını açma
    @Override
    public void turnON() {
        state = State.ON;
        System.out.println("Bluetooth: Turning ON");
        protocol.write("turnON");
    }

    // Bluetooth cihazını kapama
    @Override
    public void turnOFF() {
        state = State.OFF;
        System.out.println("Bluetooth: Turning OFF");
        protocol.write("turnOFF");
    }
}
