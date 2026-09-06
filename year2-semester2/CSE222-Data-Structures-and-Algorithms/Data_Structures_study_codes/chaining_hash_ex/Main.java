import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.LinkedList;
import java.util.Scanner;

class HashTable {
    private static class Entry {
        String key;
        String value;

        Entry(String key, String value) {
            this.key = key;
            this.value = value;
        }

        @Override
        public String toString() {
            return key + " -> " + value;
        }
    }

    private LinkedList<Entry>[] table;
    private int size;
    private int count;

    public HashTable(int size) {
        this.size = size;
        this.table = new LinkedList[size];
        for (int i = 0; i < size; i++) {
            table[i] = new LinkedList<>();
        }
        this.count = 0;
    }

    private int hashFunction(String key) {
        int hash = 0;
        for (char c : key.toCharArray()) {
            hash += c;
        }
        return hash % size;
    }

    public boolean insert(String key, String value) {
        int index = hashFunction(key);
        table[index].add(new Entry(key, value)); // Her zaman bağlı listeye ekle
        count++;
        return true;
    }

    public void display() {
        System.out.println("\nHash Tablosu:");
        for (int i = 0; i < size; i++) {
            if (table[i].isEmpty()) {
                System.out.println("[" + i + "]: Boş");
            } else {
                System.out.print("[" + i + "]: ");
                for (Entry entry : table[i]) {
                    System.out.print(entry + " | ");
                }
                System.out.println();
            }
        }
        System.out.println();
    }

    public static HashTable readInputFile(String filename) {
        HashTable hashTable = new HashTable(11); // Asal sayı boyutu tercih edilir
        try (BufferedReader br = new BufferedReader(new FileReader(filename))) {
            String line;
            while ((line = br.readLine()) != null) {
                line = line.trim();
                if (!line.isEmpty()) {
                    String[] parts = line.split(":");
                    if (parts.length == 2) {
                        hashTable.insert(parts[0].trim(), parts[1].trim());
                    }
                }
            }
            System.out.println("input.txt dosyasından veriler yüklendi.");
            hashTable.display();
        } catch (IOException e) {
            System.out.println("input.txt dosyası bulunamadı veya okunamadı!");
        } catch (Exception e) {
            System.out.println("input.txt dosyası yanlış formatta!");
        }
        return hashTable;
    }
}

public class Main {
    public static void main(String[] args) {
        HashTable hashTable = HashTable.readInputFile("input.txt");
        Scanner scanner = new Scanner(System.in);

        while (true) {
            System.out.print("Komut girin (örn: add key value, exit): ");
            String[] command = scanner.nextLine().trim().split("\\s+");

            if (command.length == 0) {
                continue;
            }

            if (command[0].equalsIgnoreCase("exit")) {
                System.out.println("Programdan çıkılıyor...");
                break;
            } else if (command[0].equalsIgnoreCase("add") && command.length == 3) {
                String key = command[1];
                String value = command[2];
                if (hashTable.insert(key, value)) {
                    System.out.println("'" + key + ": " + value + "' eklendi.");
                    hashTable.display();
                } else {
                    System.out.println("Ekleme başarısız!");
                }
            } else {
                System.out.println("Geçersiz komut! Örnek: 'add apple 5' veya 'exit'");
            }
        }

        scanner.close();
    }
}