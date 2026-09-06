package Devices;

import Protocols.Protocol;

public class Wifi extends WirelessIO {

    // Wifi cihazının constructorı
    public Wifi(Protocol protocol) {
        super("Wifi", protocol); 
    }

   
    @Override
    public void sendData(String data) {
        System.out.println("Wifi: Sending \"" + data + "\".");
        protocol.write(data);  // Protocol üzerinden veri gönderme
    }

   
    @Override
    public String recvData() {
        String data = protocol.read();  // Protocol üzerinden veri alma
        System.out.println("Wifi: Receiving data.");
        return data;
    }

    
    @Override
    public State getState() {
        return state;  // Durumu döndürme
    }

  
    @Override
    public void turnON() {
        state = State.ON;
        System.out.println("Wifi: Turning ON");
        protocol.write("turnON");
    }

    // Wifi cihazını kapama
    @Override
    public void turnOFF() {
        state = State.OFF;
        System.out.println("Wifi: Turning OFF");
        protocol.write("turnOFF");
    }
}
