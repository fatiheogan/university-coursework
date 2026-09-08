module full_adder (
	input a, b, c,
	output s, cout
);


// Define intermediate signals
wire a_plus_b, adder1_cout, adder2_cout;

// Structural
half_adder adder1 (
	.a(a),
	.b(b),
	.s(a_plus_b),
	.cout(adder1_cout)
);

half_adder adder2 (
	.a(a_plus_b),
	.b(c),
	.s(s),
	.cout(adder2_cout)
);

//Dataflow
assign cout = adder1_cout | adder2_cout;


endmodule