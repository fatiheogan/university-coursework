interface Hayvan{
    void ses();
    void yas();
}

   class Kopek implements Hayvan{
        public void ses(){
            System.out.println("Hav hav!");
        } 
        public void yas(){
            System.out.println("Age is 21");
        }
        public void food(){
            System.out.println("Kopek eats bones!");
        }

    }

     class Kedi implements Hayvan{
        public void ses(){
            System.out.println("Miyav!");
        } 
        public void yas(){
            System.out.println("Age is 23");
        }

    }

   public class Main{
    public static void main(String Args[]){

        Hayvan kedi = new Kedi();
        Kopek kopek1= new Kopek();

        kedi.yas();
        kedi.ses();
        kopek1.yas();
        kopek1.ses();
        kopek1.food();


    }
}

