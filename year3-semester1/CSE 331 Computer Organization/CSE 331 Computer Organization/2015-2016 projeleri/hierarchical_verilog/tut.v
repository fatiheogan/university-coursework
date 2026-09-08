module tut ();
	reg x, y;
	wire a, b, c;
	
	circuit testme (x, y, a, b, c);
	initial begin $monitor(
		"x=%b y=%b a=%b b=%b c=%b", x,y,a,b,c); 
		x = 0;
		y = 0;
		#3  x = 0;
		#3  y = 1;
		#3  x = 1;
		#3  y = 0;
		#3  x = 1;
		#3  y = 1;
	end	
endmodule