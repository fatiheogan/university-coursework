module EqualBinaryNumberTest();
	reg [4:0] number1,number2;
	wire result;
	equalBinaryNumber isEqual(result,number1,number2);
	initial begin
	number1= 5'b00000; number2=5'b00000;
	#20;
	number1= 5'b00001; number2=5'b00000;
	#20;
	number1= 5'b11111; number2=5'b11111;
	end
	initial begin
	$monitor("time=%2d,number1=%1b,number2=%1b,result=%1b",$time,number1,number2,result);
	end
endmodule