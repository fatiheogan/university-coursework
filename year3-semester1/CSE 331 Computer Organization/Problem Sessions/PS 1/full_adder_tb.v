// Set time resultion. (Delays are in ns, simulation resolution is in ps)
// Just keep it as is.
`timescale 1 ns / 1 ps

module full_adder_tb ();


// Define input/output signals
reg a, b, c;	// These are reg, as they are assigned in behavioral blocks.
wire s, cout;  // These are wire, as they are directly wired to module signals.


// Instantiate module
full_adder adder (
	.a(a),
	.b(b),
	.c(c),
	.s(s),
	.cout(cout)
);


// Tests (Behavioral model)
integer errors;
initial begin
	errors = 0;
	
	
	/* 		First test			*/
	// Give input.
	a = 1'b0;
	b = 1'b0;
	c = 1'b0;

	// 10 ns delay
	#10
	
	// Get output.
	if (s == 1'b0 && cout == 1'b0) begin
		$display("Test 1: success");
	end else begin
		$display("Test 1: fail");
		errors = errors + 1;
	end

	
	/* 		Second test			*/
	// Give input.
	a = 1'b1;
	b = 1'b0;
	c = 1'b0;

	// 10 ns delay
	#10 
	
	// Get output.
	if (s == 1'b1 && cout == 1'b0) begin
		$display("Test 1: success");
	end else begin
		$display("Test 1: fail");
		errors = errors + 1;
	end

	
	/* 		Third test			*/
	// Give input.
	a = 1'b1;
	b = 1'b1;
	c = 1'b0;

	// 10 ns delay
	#10 
	
	// Get output.
	if (s == 1'b0 && cout == 1'b1) begin
		$display("Test 1: success");
	end else begin
		$display("Test 1: fail");
		errors = errors + 1;
	end

	
	/* 		Forth test			*/
	// Give input.
	a = 1'b1;
	b = 1'b1;
	c = 1'b1;

	// 10 ns delay
	#10 
	
	// Get output.
	if (s == 1'b1 && cout == 1'b1) begin
		$display("Test 1: success");
	end else begin
		$display("Test 1: fail");
		errors = errors + 1;
	end

	
	/* 		Fifth test			*/
	// Give input.
	a = 1'b1;
	b = 1'b0;
	c = 1'b1;

	// 10 ns delay
	#10;
	
	// Get output.
	if (s == 1'b0 && cout == 1'b1) begin
		$display("Test 1: success");
	end else begin
		$display("Test 1: fail");
		errors = errors + 1;
	end
	
	
	// Display the total number of errors.
	$display("Errors: %d", errors);
end


endmodule
