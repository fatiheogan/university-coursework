module lab4(


	input 		          		CLOCK_50,


	input 		     [3:0]		KEY,
	input 		          		RESET_N,


	output		     [9:0]		LEDR,

	input 		     [9:0]		SW
);

wire [31:0] countVal;
wire sel;

//assign LEDR = SW;
//assign LEDR[3:0] = KEY;

//assign LEDR = countVal[31:22];

assign sel = KEY[1];  
assign LEDR = sel ? countVal[31:22] : countVal[21:12];

counter c1(KEY[0],SW[0],CLOCK_50, countVal);

//wire rst_n, d, en;
//wire q1, q2;
//
//assign rst_n = KEY[0];
//assign d = SW[0];
//assign en = SW[1];
//
//assign LEDR[0] = q1;
//assign LEDR[1] = q2;
//
//d_ff d1(en, CLOCK_50, d, q1);
//
//d_ff_full d2(en, CLOCK_50, d, rst_n, q2);

endmodule

// counter module definition
module counter (input reset_n, en, clk, output reg [31:0] count);
	
	always@(posedge clk or negedge reset_n) 
	begin
		if (!reset_n)
			count = 32'b0;
		else begin
			if (en) 
				count = count + 32'b1;
		end
	end
	
endmodule 

// D flip-flop definition
module d_ff(input en, clk, d, output reg q);

	always@(posedge clk) 
	begin
		if (en)
			q <= d;
	end

endmodule

// D flip-flop with reset definition
module d_ff_full(input en, clk, d, reset_n, output reg q);

	always@(posedge clk or negedge reset_n) 
	begin
		
		if (!reset_n)
			q <= 1'b0;
		
		else begin
			if (en)
				q <= d;
		end
		
	end




endmodule


