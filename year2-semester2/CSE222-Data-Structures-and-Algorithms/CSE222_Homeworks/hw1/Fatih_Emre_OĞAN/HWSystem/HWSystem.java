package HWSystem;

import java.io.File;
import java.util.ArrayList;
import java.util.Scanner;
import Devices.Device;
import Devices.TempSensor;

public class HWSystem {
    private ArrayList<Device> devices;
    private String[] ports; // Port bilgilerini saklamak için dizi kullanıyoruz

    public HWSystem(String configFile) {
        devices = new ArrayList<>();
        ports = new String[]{"I2C", "SPI", "OneWire", "UART"}; // Portları dizide saklıyoruz
    }

    public ArrayList<Device> getDevices() {
        return devices;
    }

    public void addDevice(Device device, int portID) {
        if (device != null && portID >= 0 && portID < ports.length) {
            device.setPortID(portID);
            devices.add(device);
            System.out.println("Device added: " + device.getName() + " to port " + portID);
        } else {
            System.out.println("Invalid device or port ID. Could not add.");
        }
    }

    public void turnOnDevice(Device device) {
        if (devices.contains(device)) {
            device.turnON();
        } else {
            System.out.println("Device not found!");
        }
    }

    public void turnOffDevice(Device device) {
        if (devices.contains(device)) {
            device.turnOFF();
        } else {
            System.out.println("Device not found!");
        }
    }

    public void readSensorData(int portID) {
        for (int i = 0; i < devices.size(); i++) {
            Device device = devices.get(i);
            if (device.getPortID() == portID) {
                if (device instanceof TempSensor) {
                    if (device.getState() == Devices.State.ON) {
                        float temp = ((TempSensor) device).getTemp();
                        System.out.println("Temperature reading from port " + portID + ": " + temp + "°C");
                    } else {
                        System.out.println("Device is not active.");
                    }
                    return;
                }
            }
        }
        System.out.println("No device found on port " + portID);
    }

    public void listSensors() {
        System.out.println("list of Sensors:");
        boolean found = false;
        for (int i = 0; i < devices.size(); i++) {
            Device device = devices.get(i);
            if (device instanceof TempSensor) {
                found = true;
                System.out.println(device.getName() + " " + device.getDevID() + " " + device.getPortID() + " " + ports[device.getPortID()]);
            }
        }
        if (!found) {
            System.out.println("No sensors found.");
        }
    }

    public void rmDevice(int portID) {
        for (int i = 0; i < devices.size(); i++) {
            Device device = devices.get(i);
            if (device.getPortID() == portID) {
                if (device.getState() == Devices.State.ON) {
                    System.out.println("Device is active, cannot be removed.");
                    return;
                } else {
                    devices.remove(i);
                    System.out.println("Device removed from port " + portID);
                    return;
                }
            }
        }
        System.out.println("No device found on port " + portID);
    }

    public void listPorts() {
        System.out.println("list of ports:");
        for (int i = 0; i < ports.length; i++) {
            String status = "empty";
            String deviceInfo = "";

            for (int j = 0; j < devices.size(); j++) {
                Device device = devices.get(j);
                if (device.getPortID() == i) {
                    status = "occupied";
                    if (device instanceof TempSensor) {
                        TempSensor sensor = (TempSensor) device;
                        deviceInfo = device.getName() + " TempSensor Sensor " + sensor.getTemp() + " " + device.getState();
                    } else {
                        deviceInfo = device.getName() + " " + device.getDevType() + " " + device.getState();
                    }
                    break;
                }
            }
            
            if (!deviceInfo.isEmpty()) {
                System.out.println(i + " " + ports[i] + " " + status + " " + deviceInfo);
            } else {
                System.out.println(i + " " + ports[i] + " " + status);
            }
        }
    }

    public void exitSystem() {
        System.out.println("Exiting system...");
    }
}
