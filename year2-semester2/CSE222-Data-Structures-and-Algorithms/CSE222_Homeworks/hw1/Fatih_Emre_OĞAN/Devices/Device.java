package Devices;

import Protocols.Protocol;

public abstract class Device {
    private static int idCounter = 0;  // Benzersiz ID için sayaç
    private final int id;  // Cihazın benzersiz kimliği

    protected Protocol protocol;
    protected State state;
    protected String name;
    protected String devType;
    protected int portID;

    // Constructor
    public Device(String name, String devType) {
        this.name = name;
        this.devType = devType;
        this.id = idCounter++;  // Her cihaz oluşturulduğunda benzersiz bir ID veriliyor
        this.state = State.OFF;  // Varsayılan olarak OFF
    }
    public int getID() {
        return this.id;
    }

    
    public int getDevID() {
        return id;  
    }

    // Port ID'sini almak için getter
    public int getPortID() {
        return this.portID;
    }

    // Port ID'sini ayarlamak için setter
    public void setPortID(int portID) {
        this.portID = portID;
    }

    // Cihazı açma işlemi
    public abstract void turnON();

    // Cihazı kapama işlemi
    public abstract void turnOFF();

    // Durum bilgisi almak için metod
    public State getState() {
        return this.state;
    }

    // Cihaz adı almak için metod
    public String getName() {
        return this.name;
    }

    // Cihaz türünü almak için metod
    public String getDevType() {
        return this.devType;
    }
}
