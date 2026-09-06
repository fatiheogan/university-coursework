//=======================================================
//  This code is corrected from the original Terasic System Builder code
//=======================================================

module lab4_1 (
    input              CLOCK_50,
    input  [3:0]       KEY,
    output [9:0]       LEDR,
    input  [9:0]       SW
);

    wire b = ~KEY[0];
    wire [31:0] cnt;

    circuit c1 (
        .b(b),
        .clk(CLOCK_50),
        .led(LEDR[2:0])
    );

    counter c0 (
        .clk(CLOCK_50),
        .reset_n(KEY[0]),
        .en(1'b1),
        .count(cnt)
    );

endmodule

module circuit (
    input b,
    input clk,
    output [2:0] led
);

    reg [1:0] s;
    wire [1:0] n;

    always @(posedge clk) 
    begin
        s <= n;
    end
    
    assign n[1] = (s[0] & ~s[1] & b) | (~s[0] & s[1] & ~b);
    assign n[0] = (~s[1]) & (s[0] ^ b);
	 
    assign led[0] = ~s[0];
    assign led[1] = (~s[1]) & (s[0] ^ b);
    assign led[2] = (s[0] & ~s[1] & b) | (~s[0] & s[1] & ~b);
    
endmodule

module counter (
    input clk,
    input reset_n,
    input en,
    output reg [31:0] count
);
    
    always @(posedge clk or negedge reset_n) 
    begin
        if (!reset_n)
            count <= 32'b0;
        else if (en) 
            count <= count + 32'b1;
    end
    
endmodule