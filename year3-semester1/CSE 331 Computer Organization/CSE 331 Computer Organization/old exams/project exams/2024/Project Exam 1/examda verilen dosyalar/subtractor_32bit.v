module subtractor_32bit (
    input [31:0] a,
    input [31:0] b,
    output [31:0] diff
);
    wire [31:0] b_complement;
    wire [31:0] carry;

    // Invert `b` for 2's complement subtraction (b_complement = ~b + bin).
    not (b_complement[0], b[0]);
    not (b_complement[1], b[1]);
    not (b_complement[2], b[2]);
    not (b_complement[3], b[3]);
    not (b_complement[4], b[4]);
    not (b_complement[5], b[5]);
    not (b_complement[6], b[6]);
    not (b_complement[7], b[7]);
    not (b_complement[8], b[8]);
    not (b_complement[9], b[9]);
    not (b_complement[10], b[10]);
    not (b_complement[11], b[11]);
    not (b_complement[12], b[12]);
    not (b_complement[13], b[13]);
    not (b_complement[14], b[14]);
    not (b_complement[15], b[15]);
    not (b_complement[16], b[16]);
    not (b_complement[17], b[17]);
    not (b_complement[18], b[18]);
    not (b_complement[19], b[19]);
    not (b_complement[20], b[20]);
    not (b_complement[21], b[21]);
    not (b_complement[22], b[22]);
    not (b_complement[23], b[23]);
    not (b_complement[24], b[24]);
    not (b_complement[25], b[25]);
    not (b_complement[26], b[26]);
    not (b_complement[27], b[27]);
    not (b_complement[28], b[28]);
    not (b_complement[29], b[29]);
    not (b_complement[30], b[30]);
    not (b_complement[31], b[31]);

    // Perform a + ~b + bin
    adder_32bit adder_inst (.a(a), .b(b_complement), .cin(1'b0), .sum(diff));
endmodule