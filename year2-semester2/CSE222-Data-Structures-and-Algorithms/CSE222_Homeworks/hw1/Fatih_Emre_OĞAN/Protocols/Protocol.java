package Protocols;

public interface Protocol {
    String getProtocolName();  // Protokol adını döndürür 
    
    String read();  // Veri okuma metodu
    
    void write(String data);  // Veri yazma metodu

    // I2C protokolüne özel metod. 
    default void writeToI2C(int address, String command) {
        
    }
}
