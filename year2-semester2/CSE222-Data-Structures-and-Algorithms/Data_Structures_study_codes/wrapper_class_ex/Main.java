public class Main {
    public static void main(String[] args) {
        // İlkel tip
        int primitiveInt = 42;
        System.out.println("Primitive int: " + primitiveInt);

        // Wrapper sınıf ile nesne
        Integer wrapperInt = new Integer(42); // Eski yöntem (depreciated)
        Integer autoWrapper = 42; // Autoboxing ile modern yöntem
        System.out.println("Wrapper Integer: " + wrapperInt);

        // İlkel tipi wrapper'a çevirme (autoboxing)
        Integer boxedInt = primitiveInt; // Otomatik olarak Integer'a çevrilir
        System.out.println("Boxed Integer: " + boxedInt);

        // Wrapper'dan ilkel tipe çevirme (unboxing)
        int unboxedInt = wrapperInt; // Otomatik olarak int'e çevrilir
        System.out.println("Unboxed int: " + unboxedInt);

        // Wrapper sınıfın metodlarını kullanma
        String intAsString = Integer.toString(42);
        System.out.println("Integer as String: " + intAsString);
    }
}