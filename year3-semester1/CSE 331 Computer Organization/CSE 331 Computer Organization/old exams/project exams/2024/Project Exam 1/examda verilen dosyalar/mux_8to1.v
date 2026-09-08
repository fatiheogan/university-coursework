module mux_8to1 (
    input [31:0] a,    // Input 0
    input [31:0] b,    // Input 1
    input [31:0] c,    // Input 2
    input [31:0] d,    // Input 3
    input [31:0] e,    // Input 4
    input [31:0] f,    // Input 5
    input [31:0] g,    // Input 6
    input [31:0] h,    // Input 7
    input [2:0] sel,   // 3-bit select signal
    output [31:0] out  // Output
);

    // Using 'assign' to select the appropriate input based on the 'sel' signal
    assign out = (sel == 3'b000) ? a :
                 (sel == 3'b001) ? b :
                 (sel == 3'b010) ? c :
                 (sel == 3'b011) ? d :
                 (sel == 3'b100) ? e :
                 (sel == 3'b101) ? f :
                 (sel == 3'b110) ? g : h;

endmodule