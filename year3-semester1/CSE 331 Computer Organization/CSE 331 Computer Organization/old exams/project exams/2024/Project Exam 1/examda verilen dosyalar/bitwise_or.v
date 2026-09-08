module bitwise_or (
    input [31:0] a,
    input [31:0] b,
    output [31:0] result
);
    or (result[0], a[0], b[0]);
    or (result[1], a[1], b[1]);
    or (result[2], a[2], b[2]);
    or (result[3], a[3], b[3]);
    or (result[4], a[4], b[4]);
    or (result[5], a[5], b[5]);
    or (result[6], a[6], b[6]);
    or (result[7], a[7], b[7]);
    or (result[8], a[8], b[8]);
    or (result[9], a[9], b[9]);
    or (result[10], a[10], b[10]);
    or (result[11], a[11], b[11]);
    or (result[12], a[12], b[12]);
    or (result[13], a[13], b[13]);
    or (result[14], a[14], b[14]);
    or (result[15], a[15], b[15]);
    or (result[16], a[16], b[16]);
    or (result[17], a[17], b[17]);
    or (result[18], a[18], b[18]);
    or (result[19], a[19], b[19]);
    or (result[20], a[20], b[20]);
    or (result[21], a[21], b[21]);
    or (result[22], a[22], b[22]);
    or (result[23], a[23], b[23]);
    or (result[24], a[24], b[24]);
    or (result[25], a[25], b[25]);
    or (result[26], a[26], b[26]);
    or (result[27], a[27], b[27]);
    or (result[28], a[28], b[28]);
    or (result[29], a[29], b[29]);
    or (result[30], a[30], b[30]);
    or (result[31], a[31], b[31]);
endmodule