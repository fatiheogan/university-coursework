module mux_demo ();
	reg [1:0] selector;
	reg [3:0] adder;
	reg [3:0] subtractor;
	reg [3:0] andop;
	reg [3:0] xorop;
	wire [3:0] fout;
	
	mux_full testme (fout, selector, adder, subtractor, andop, xorop);
	initial begin
		//"x=%b y=%b a=%b b=%b c=%b", x,y,a,b,c); 
		adder[0] = 1; adder[1] = 1; adder[2] = 1; adder[3] = 1;
		subtractor[0] = 0; subtractor[1] = 0; subtractor[2] = 0; subtractor[3] = 0;
		andop[0] = 1; andop[1] = 0; andop[2] = 1; andop[3] = 0;
		xorop[0] = 0; xorop[1] = 0; xorop[2] = 1; xorop[3] = 1;
		
		selector[1] = 0; selector[0] = 0;
		#20  selector[1] = 0; selector[0] = 1;
		#20  selector[1] = 1; selector[0] = 0;
		#20  selector[1] = 1; selector[0] = 1;
	end	
endmodule