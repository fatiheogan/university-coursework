package Devices;

import Protocols.Protocol;

public abstract class Display extends Device {

    // Protocol parametreli constructor
    public Display(String name, Protocol protocol) {
        super(name, "Display");  // Burada devType'ı Display olarak manuel şekilde veriyoruz
        this.protocol = protocol;  // Protocol'ü doğrudan Display sınıfında ayarlıyoruz
        this.state = State.OFF;  // Varsayılan olarak OFF durumu
    }
    

    // Display için veri yazdırma metodu
    public abstract void printData(String data);
}
