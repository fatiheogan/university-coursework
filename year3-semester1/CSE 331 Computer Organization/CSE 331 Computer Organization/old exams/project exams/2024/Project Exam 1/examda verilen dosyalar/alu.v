module alu (
    input [31:0] a,        // First input
    input [31:0] b,        // Second input
    input [3:0] alu_op,    // ALU operation code (4-bit)
    output [31:0] result  // ALU result

);

    // Internal wires for various ALU operations
    wire [31:0] sum, diff, and_out, or_out, xor_out, nor_out;
    

    // Perform addition, subtraction, and logical operations
    adder_32bit add (a, b, 1'b0, sum);                  // 32-bit adder
    subtractor_32bit sub (a, b, diff);              // 32-bit subtractor
    bitwise_and and_op (a, b, and_out);             // AND operation
    bitwise_or or_op (a, b, or_out);                // OR operation
    bitwise_xor xor_op (a, b, xor_out);             // XOR operation
    bitwise_nor nor_op (a, b, nor_out);             // NOR operation
    
    // 8-to-1 MUX to select the final result based on ALU operation
    mux_8to1 mux (
        .a(sum), 
        .b(diff), 
        .c(and_out), 
        .d(or_out), 
        .e(xor_out), 
        .f(nor_out), 
        .g(32'b0), //Not used
        .h(32'b0), //Not used 
        .sel(alu_op),
        .out(result)
    );

endmodule