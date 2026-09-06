// Ana Modül
module lab5 
(
    input CLOCK_50,
    input [1:0] KEY, // KEY[0]: Artırma, KEY[1]: Azaltma (aktif düşük)
    output [9:0] LEDR
);
    wire increase = ~KEY[0]; // Artırma butonu (aktif düşük)
    wire decrease = ~KEY[1]; // Azaltma butonu (aktif düşük)
    wire [31:0] cnt;
    wire slow_clk = cnt[25]; // ~0.745 Hz (50 MHz / 2^26)

    counter c0 (.clk(CLOCK_50), .count(cnt));
    circuit c1 (.increase(increase), .decrease(decrease), .clk(slow_clk), .led(LEDR[2:0]));
    assign LEDR[9:3] = 7'b0; // Kullanılmayan LED'ler kapalı
endmodule

// Durum Makinesi Modülü
module circuit 
(
    input increase,
    input decrease,
    input clk,
    output [2:0] led
);
    reg [1:0] s; // Mevcut durum
    wire [1:0] n; // Sonraki durum

    // Durum register'ı (sıfırlama ile)
    always @(posedge clk) begin
        s <= n;
    end

    // Sonraki durum mantığı (daha sadeleştirilmiş)
    assign n = (increase & ~decrease) ? s + 2'b1 : // Artırma
               (~increase & decrease) ? s - 2'b1 : // Azaltma
               s; // Durumu koru

    // Çıkış mantığı
    assign led[1:0] = s; // Mevcut sayıyı temsil eder
    assign led[2] = s[0]; // Teklik LED'i
endmodule

// Sayaç Modülü
module counter 
(
    input clk,
    output reg [31:0] count
);
    initial begin
        count = 32'b0; // Başlangıç değeri
    end

    always @(posedge clk) begin
        count <= count + 32'b1;
    end
endmodule