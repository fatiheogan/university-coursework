package Devices;

import Protocols.Protocol;

public abstract class WirelessIO extends Device {

    // WirelessIO cihazının constructorı
    public WirelessIO(String name, Protocol protocol) {
        super(name, "WirelessIO");  // "WirelessIO" devType olarak belirtiliyor
        this.protocol = protocol;  // Protocol'ü bu sınıfa aktarıyoruz
    }

    // WirelessIO sınıfı için veri gönderme işlemi
    public abstract void sendData(String data);

    // WirelessIO sınıfı için veri alma işlemi
    public abstract String recvData();
}
