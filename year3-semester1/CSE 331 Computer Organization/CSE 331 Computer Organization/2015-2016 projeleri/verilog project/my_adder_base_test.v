module my_adder_base_test();
	reg a,b,cin;
	wire sum,cout;
	
	my_adder_base testing(a,b,cin,sum,cout);
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