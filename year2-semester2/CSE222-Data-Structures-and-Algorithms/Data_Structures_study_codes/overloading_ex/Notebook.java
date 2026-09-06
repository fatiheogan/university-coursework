 public class Notebook {
    // Sabit varsayılan üretici
    private static final String DEFAULT_NB_MAN = "Dell"; // Sabit bir değer

    // Özellikler (fields)
    private String manufacturer;
    private String processor;
    private double ram;
    private int disk;
    private double procSpeed;
    private double screenSize;
    private double weight;

    // 7 parametreli constructor
    public Notebook(String man, String processor, double ram, int disk, 
                    double procSpeed, double screen, double wei) {
        this.manufacturer = man;
        this.processor = processor;
        this.ram = ram;
        this.disk = disk;
        this.procSpeed = procSpeed;
        this.screenSize = screen;
        this.weight = wei;
    }

    // 6 parametreli constructor (overloaded)
    public Notebook(String processor, double ram, int disk, 
                    double procSpeed, double screen, double wei) {
        // 7 parametreli constructor'ı çağırıyor, üretici olarak DEFAULT_NB_MAN kullanıyor
        this(DEFAULT_NB_MAN, processor, ram, disk, procSpeed, screen, wei);
    }

    // Özellikleri görmek için toString metodu
    @Override
    public String toString() {
        return "Manufacturer: " + manufacturer + "\n" +
               "Processor: " + processor + "\n" +
               "RAM: " + ram + " GB\n" +
               "Disk: " + disk + " GB\n" +
               "Speed: " + procSpeed + " GHz\n" +
               "Screen Size: " + screenSize + " inches\n" +
               "Weight: " + weight + " pounds";
    }

    // Test için main metodu
    public static void main(String[] args) {
        // 7 parametreli constructor ile nesne
        Notebook fullSpec = new Notebook("HP", "Intel", 8, 512, 2.6, 15.6, 4.5);
        System.out.println("Full Spec Notebook:\n" + fullSpec);

        System.out.println("-------------------");

        // 6 parametreli constructor ile nesne (varsayılan üretici: Dell)
        Notebook defaultSpec = new Notebook("AMD", 4, 256, 1.8, 14.0, 3.5);
        System.out.println("Default Spec Notebook:\n" + defaultSpec);
    }
}
