module test_module();
	reg a,b,cin;
	wire sum,cout;
	
	my_adder testing(a,b,cin,sum,cout);
	initial begin $monitor( "sum = %b cout = %b", sum,cout );
		a = 0; b = 0; cin = 0;
		#100
		a = 0; b = 0; cin = 1;
		#100
		a = 0; b = 1; cin = 0;
		#100
		a = 0; b = 1; cin = 1;
		#100
		a = 1; b = 0; cin = 0;
		#100
		a = 1; b = 0; cin = 1;
		#100
		a = 1; b = 1; cin = 0;
		#100
		a = 1; b = 1; cin = 1;
	end
endmodule

00000
00001
00010
00011
00100
00101
00110
00111
01000
01001
01010
01011
01100
01101
01110
01111
10000
10001
10010
10011
10100
10101
10110
10111
11000
11001
11010
11011
11100
11101
11110
11111