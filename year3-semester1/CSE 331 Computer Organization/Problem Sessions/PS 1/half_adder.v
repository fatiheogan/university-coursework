module half_adder (
	input a, b,
	output s, cout
);


//Dataflow
assign s = a ^ b;
assign cout = a & b;


endmodule