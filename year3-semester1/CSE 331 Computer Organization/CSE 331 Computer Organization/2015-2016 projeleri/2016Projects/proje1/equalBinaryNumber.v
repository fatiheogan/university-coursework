module equalBinaryNumber(result,number1[4:0],number2[4:0]);
	
	input [4:0] number1, number2;
	output result;
	wire [3:0] xorResult;
	wire [1:0] tempOperations;
	wire tempResultAnd;
	wire tempResultFirstBitsAnd;
	wire notFirstBit;
	
	xor
		n1 (xorResult[3],number1[3],number2[3]),
		n2 (xorResult[2],number1[2],number2[3]),
		n3 (xorResult[1],number1[1],number2[1]),
		n4 (xorResult[0],number1[0],number2[0]);
	
	and
		n5 (tempResultAnd,xorResult[0],xorResult[1],xorResult[2],xorResult[3]);
	and
		n6 (tempResultFirstBitsAnd,number1[4],number2[4]);
	not
		n7 (notFirstBit,tempResultFirstBitsAnd);
	and
		n8 (result,tempResultAnd,notFirstBit);
	
endmodule