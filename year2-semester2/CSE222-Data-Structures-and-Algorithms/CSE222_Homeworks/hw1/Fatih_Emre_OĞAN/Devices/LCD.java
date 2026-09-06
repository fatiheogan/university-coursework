package Devices;

import Protocols.Protocol;

public class LCD extends Display {

    // LCD ekranın adı ve durumu
    private String devName = "LCD";
    private State state;

   
    public LCD(Protocol protocol) {
        super("LCD", protocol);  
        this.state = State.OFF; // Varsayılan olarak OFF durumu
    }

    // Display sınıfından devralınan turnON metodu
    @Override
    public void turnON() {
        System.out.println("LCD: Turning ON");
        protocol.write("turnON"); // Protocol üzerinden veriyi gönderme
        this.state = State.ON; // Durum değiştirildi
    }

    
    @Override
    public void turnOFF() {
        System.out.println("LCD: Turning OFF");
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

    // LCD ekranına veri yazdırma işlemi
    @Override
    public void printData(String data) {
        System.out.println("LCD: Writing \"" + data + "\".");
        protocol.write(data); // Protocol üzerinden veri yazdırma
    }
}
