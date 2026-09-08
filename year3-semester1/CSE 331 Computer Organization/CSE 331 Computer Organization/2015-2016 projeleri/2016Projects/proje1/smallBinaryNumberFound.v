module smallBinaryNumberFound(result,number1[4:0],number2[4:0]);
	input [4:0] number1, number2;
	output result;
	//wire tempFirstBitsAnding[2:0],tempBetweenBitsAnding[3:0],number1Not[3:0],xorResult[2:0];
	wire [2:0] xorResult;
	wire [1:0] tempFirstBitsAnding;
	wire [3:0] tempBetweenBitsAnding,number1Not;
	wire tempResultFirstBitsAnd;
	wire tempResult;
	wire notFirstBit;
	wire temp,tempNot;
	xor
		n1 (xorResult[2],number1[3],number2[3]),
		n2 (xorResult[1],number1[2],number2[2]),
		n3 (xorResult[0],number1[1],number2[1]);

	and 
		n5 (tempFirstBitsAnding[0],xorResult[2],xorResult[1]),
		n6 (tempFirstBitsAnding[1],xorResult[2],xorResult[1],xorResult[0]);
	
	not
		n7 (number1Not[0],number1[0]),
		n8 (number1Not[1],number1[1]),
		n9 (number1Not[2],number1[2]),
		n10 (number1Not[3],number1[3]);
		
	and 
		n11 (tempBetweenBitsAnding[0],number2[3],number1Not[3]),
		n12 (tempBetweenBitsAnding[1],number2[2],number1Not[2],xorResult[2]),
		n13 (tempBetweenBitsAnding[2],number2[1],number1Not[1],tempFirstBitsAnding[0]),
		n14 (tempBetweenBitsAnding[3],number2[0],number1Not[0],tempFirstBitsAnding[1]);
	
	and
		n15 (tempResultFirstBitsAnd,number1[4],number2[4]);
	not
		n16 (notFirstBit,tempResultFirstBitsAnd);
	
	or 
		n17 (tempResult,tempBetweenBitsAnding[0],tempBetweenBitsAnding[1],tempBetweenBitsAnding[2],tempBetweenBitsAnding[3]);

	and
		n18 (result,tempResult,notFirstBit);

endmodule