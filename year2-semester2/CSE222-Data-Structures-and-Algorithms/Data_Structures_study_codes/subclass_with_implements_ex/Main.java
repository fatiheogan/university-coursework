// Abstract üst sınıf
abstract class Vehicle {
    protected String brand;
    protected int speed;

    public Vehicle(String brand, int speed) {
        this.brand = brand;
        this.speed = speed;
    }

    // Somut metod
    public void move() {
        System.out.println(brand + " " + speed + " km/h hızla hareket ediyor.");
    }

    // Soyut metod (alt sınıflar uygulamalı)
    public abstract void describe();
}

// Interface
interface FuelEfficient {
    double calculateFuelEfficiency(); // Soyut metod
    default void ecoMode() {          // Somut default metod
        System.out.println("Eco modu aktif.");
    }
}

// Hem extends hem implements yapan sınıf
class HybridCar extends Vehicle implements FuelEfficient {
    private double batteryLevel; // Ek özellik

    public HybridCar(String brand, int speed, double batteryLevel) {
        super(brand, speed); // Üst sınıfın constructor’ını çağır
        this.batteryLevel = batteryLevel;
    }

    // Vehicle’dan gelen soyut metodu override et
    @Override
    public void describe() {
        System.out.println(brand + " bir hibrit araçtır, hızı: " + speed + " km/h, batarya seviyesi: " + batteryLevel + "%.");
    }

    // FuelEfficient interface’inden gelen soyut metodu implement et
    @Override
    public double calculateFuelEfficiency() {
        return speed / batteryLevel * 100; // Basit bir hesaplama
    }
}

// Test sınıfı
public class Main {
    public static void main(String[] args) {
        HybridCar car = new HybridCar("Toyota", 120, 75.0);
        car.move();                     // Vehicle’dan miras
        car.describe();                 // Vehicle’dan override
        car.ecoMode();                  // FuelEfficient’tan default metod
        System.out.println("Yakıt verimliliği: " + car.calculateFuelEfficiency() + " birim");
    }
}