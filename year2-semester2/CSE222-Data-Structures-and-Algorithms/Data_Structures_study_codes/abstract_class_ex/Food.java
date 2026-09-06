public abstract class Food {
    public final String name;
    private double calories;

    // Normal (concrete) metod
    public double getCalories() {
        return calories;
    }

    // Constructor
    protected Food(String name, double calories) {
        this.name = name;
        this.calories = calories;
    }

    // Soyut metodlar
    public abstract double percentProtein();
    public abstract double percentFat();
    public abstract double percentCarbs();
}