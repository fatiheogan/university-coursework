
module lab5_1 (
    input CLOCK_50,
    input [1:0] KEY,
    output [9:0] LEDR
);
    wire increase = ~KEY[0]; 
    wire decrease = ~KEY[1];
    wire [31:0] cnt;
    wire slow_clk = cnt[25]; 

    counter c0 (.clk(CLOCK_50), .count(cnt));
    circuit c1 (.increase(increase), .decrease(decrease), .clk(slow_clk), .led(LEDR[2:0]));
    assign LEDR[9:3] = 0; 
endmodule


module circuit (
    input increase,
    input decrease,
    input clk,
    output [2:0] led
);
    reg  [1:0] s; 
    wire [1:0] n; 

   
    always @(posedge clk) begin
        s <= n;
    end


    assign n[1] = (~s[1] & ~s[0] & ~increase & decrease) |
                  (~s[1] & s[0] & increase & ~decrease) |
                  (s[1] & ~increase & ~decrease) |
                  (s[1] & s[0] & decrease) |
                  (s[1] & ~s[0] & increase);
                  
    assign n[0] = (s[0] & ~increase & ~decrease) |
                  (~s[0] & ~increase & decrease) |
                  (s[0] & increase & decrease) |
                  (~s[0] & increase & ~decrease);

    assign led[1:0] = s[1:0]; 
    assign led[2] = s[0]; 
endmodule


module counter (
    input clk,
    output reg [31:0] count
);
    initial count = 32'b0; 
    always @(posedge clk) begin
        count <= count + 32'b1;
    end
endmodule
