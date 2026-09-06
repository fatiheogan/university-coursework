package Main;

import Protocols.*;
import Devices.*;
import HWSystem.HWSystem;
import java.util.Scanner;

public class Main {

    public static void main(String[] args) {
        // Protokoller oluşturuluyor
        Protocol uartProtocol = new UART();
        Protocol spiProtocol = new SPI();
        Protocol i2cProtocol = new I2C();
        Protocol oneWireProtocol = new OneWire();

        // HWSystem oluşturuluyor
        HWSystem hwSystem = new HWSystem("config.txt");

        // Kullanıcı komutları
        Scanner scanner = new Scanner(System.in);
        String command;

        while (true) {
            System.out.print("Command: ");
            command = scanner.nextLine();

            if (command.startsWith("list ports")) {
                hwSystem.listPorts();
            } else if (command.startsWith("list Sensor")) {  // ✅ list Sensor komutu eklendi!
                hwSystem.listSensors();
            } else if (command.startsWith("addDev")) {
                String[] parts = command.split(" ");
                if (parts.length < 4) {
                    System.out.println("Invalid command format!");
                    continue;
                }

                String deviceName = parts[1];
                int portID = Integer.parseInt(parts[2]);
                int protocolID = Integer.parseInt(parts[3]);

                Device newDevice = null;
                Protocol selectedProtocol = null;

                // Protokolü seçiyoruz
                switch (protocolID) {
                    case 0:
                        selectedProtocol = uartProtocol;
                        break;
                    case 1:
                        selectedProtocol = spiProtocol;
                        break;
                    case 2:
                        selectedProtocol = i2cProtocol;
                        break;
                    case 3:
                        selectedProtocol = oneWireProtocol;
                        break;
                    default:
                        System.out.println("Invalid protocol ID!");
                        continue;
                }

                // Cihaz türüne göre nesne oluşturuyoruz
                if (deviceName.equals("DHT11") && selectedProtocol != null) {
                    newDevice = new DHT11(selectedProtocol);
                } else if (deviceName.equals("BME280") && (selectedProtocol == spiProtocol || selectedProtocol == i2cProtocol)) {
                    newDevice = new BME280(selectedProtocol);
                } else if (deviceName.equals("MPU6050") && selectedProtocol == i2cProtocol) {
                    newDevice = new MPU6050(selectedProtocol);
                } else {
                    System.out.println("Invalid device name or protocol ID!");
                    continue;
                }

                // Cihazı sisteme ekliyoruz
                if (newDevice != null) {
                    hwSystem.addDevice(newDevice, portID);
                    System.out.println("Device added: " + newDevice.getName() + " to port " + portID);
                }
            } else if (command.startsWith("turnON")) {
                int portID = Integer.parseInt(command.split(" ")[1]);
                Device device = getDeviceByPort(portID, hwSystem);
                if (device != null) {
                    hwSystem.turnOnDevice(device);
                } else {
                    System.out.println("Device not found on port " + portID);
                }
            } else if (command.startsWith("turnOFF")) {
                int portID = Integer.parseInt(command.split(" ")[1]);
                Device device = getDeviceByPort(portID, hwSystem);
                if (device != null) {
                    hwSystem.turnOffDevice(device);
                } else {
                    System.out.println("Device not found on port " + portID);
                }
            } else if (command.startsWith("readSensor")) {
                int devID = Integer.parseInt(command.split(" ")[1]);
                Device device = getDeviceByDevID(devID, hwSystem);
                if (device instanceof TempSensor && device.getState() == Devices.State.ON) {
                    hwSystem.readSensorData(device.getPortID());
                } else {
                    System.out.println("Device is not active .");
                }
            } else if (command.startsWith("exit")) {
                hwSystem.exitSystem();
                break;
            }
        }
    }

    private static Device getDeviceByPort(int portID, HWSystem hwSystem) {
        for (Device device : hwSystem.getDevices()) {
            if (device.getPortID() == portID) {
                return device;
            }
        }
        System.out.println("No device found on port " + portID);
        return null;
    }

    private static Device getDeviceByDevID(int devID, HWSystem hwSystem) {
        for (Device device : hwSystem.getDevices()) {
            if (device.getID() == devID) {
                return device;
            }
        }
        System.out.println("No device found with devID " + devID);
        return null;
    }
}
