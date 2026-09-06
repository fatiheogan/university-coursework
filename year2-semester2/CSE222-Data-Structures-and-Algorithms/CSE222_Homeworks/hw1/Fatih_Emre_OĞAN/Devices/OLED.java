package Devices;

import Protocols.Protocol;

public class OLED extends Display {

    
    private String devName;

   
    public OLED(Protocol protocol) {
        super("OLED", protocol);  // Cihazın adı ve protocol Display sınıfına geçiriyoruz
        this.devName = "OLED";  // devName'yi burada tanımlıyoruz
        this.state = State.OFF;  // Varsayılan olarak OFF durumu
    }

    @Override
    public void turnON() {
        System.out.println("OLED: Turning ON");
        protocol.write("turnON"); // Protocol üzerinden veriyi gönderme
        this.state = State.ON; // Durum değiştirildi
    }

   
    @Override
    public void turnOFF() {
        System.out.println("OLED: Turning OFF");
        protocol.write("turnOFF"); // Protocol üzerinden veriyi gönderme
        this.state = State.OFF; // Durum değiştirildi
    }

    
    @Override
    public State getState() {
        return this.state;
    }

    
    @Override
    public String getDevType() {
        return "Display"; // Cihazın türü
    }

   
    @Override
    public String getName() {
        return this.devName; // Cihazın adı
    }

    
    @Override
    public void printData(String data) {
        System.out.println("OLED: Writing \"" + data + "\".");
        protocol.write(data); // Protocol üzerinden veri yazdırma
    }
}
