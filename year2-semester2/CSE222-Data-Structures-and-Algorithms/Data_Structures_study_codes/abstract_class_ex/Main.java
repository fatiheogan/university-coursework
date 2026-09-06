public class Main {
    public static void main(String[] args) {
        Food apple = new Apple("Red Apple", 95.0);
        System.out.println("Name: " + apple.name);
        System.out.println("Calories: " + apple.getCalories());
        System.out.println("Protein: " + apple.percentProtein() + "%");
        System.out.println("Fat: " + apple.percentFat() + "%");
        System.out.println("Carbs: " + apple.percentCarbs() + "%");
    }
}
