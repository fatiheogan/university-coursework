module bitwise_xor (
    input [31:0] a,
    input [31:0] b,
    output [31:0] result
);
    xor (result[0], a[0], b[0]);
    xor (result[1], a[1], b[1]);
    xor (result[2], a[2], b[2]);
    xor (result[3], a[3], b[3]);
    xor (result[4], a[4], b[4]);
    xor (result[5], a[5], b[5]);
    xor (result[6], a[6], b[6]);
    xor (result[7], a[7], b[7]);
    xor (result[8], a[8], b[8]);
    xor (result[9], a[9], b[9]);
    xor (result[10], a[10], b[10]);
    xor (result[11], a[11], b[11]);
    xor (result[12], a[12], b[12]);
    xor (result[13], a[13], b[13]);
    xor (result[14], a[14], b[14]);
    xor (result[15], a[15], b[15]);
    xor (result[16], a[16], b[16]);
    xor (result[17], a[17], b[17]);
    xor (result[18], a[18], b[18]);
    xor (result[19], a[19], b[19]);
    xor (result[20], a[20], b[20]);
    xor (result[21], a[21], b[21]);
    xor (result[22], a[22], b[22]);
    xor (result[23], a[23], b[23]);
    xor (result[24], a[24], b[24]);
    xor (result[25], a[25], b[25]);
    xor (result[26], a[26], b[26]);
    xor (result[27], a[27], b[27]);
    xor (result[28], a[28], b[28]);
    xor (result[29], a[29], b[29]);
    xor (result[30], a[30], b[30]);
    xor (result[31], a[31], b[31]);
endmodule