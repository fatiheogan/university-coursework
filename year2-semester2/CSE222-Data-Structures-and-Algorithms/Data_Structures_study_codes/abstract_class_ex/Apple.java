public class Apple extends Food {
    public Apple(String name, double calories) {
        super(name, calories); // Üst sınıfın constructor'ını çağır
    }

    @Override
    public double percentProtein() {
        return 0.5; // Elmanın protein yüzdesi
    }

    @Override
    public double percentFat() {
        return 0.2; // Elmanın yağ yüzdesi
    }

    @Override
    public double percentCarbs() {
        return 85.0; // Elmanın karbonhidrat yüzdesi
    }
}