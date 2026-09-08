`define R_TYPE 000000

module mips_core_testbench_tester();
	reg [31:0]mem_array[31:0];
	reg [31:0]instruction;
	reg [31:0]rs_content;
	reg [31:0]rt_content;
	wire [31:0]result;
	wire [31:0]s_inst;
	
	mux16x1 test(s_inst,4'b1111);
	
	initial begin $readmemh("registers.h",mem_array); end
	
	always @(s_inst) begin
		instruction = s_inst;
		$monitor("instruction %b",instruction);
	end
	
	always @(instruction) begin
		if(instruction[25:21] === 5'b00000)begin
			rs_content = mem_array[0];
		end else if(instruction[25:21] === 5'b00001) begin
			rs_content = mem_array[1];
		end else if(instruction[25:21] === 5'b00010) begin
			rs_content = mem_array[2];
		end else if(instruction[25:21] === 5'b00011) begin
			rs_content = mem_array[3];
		end else if(instruction[25:21] === 5'b00100) begin
			rs_content = mem_array[4];
		end else if(instruction[25:21] === 5'b00101) begin
			rs_content = mem_array[5];
		end else if(instruction[25:21] === 5'b00110) begin
			rs_content = mem_array[6];
		end else if(instruction[25:21] === 5'b00111) begin
			rs_content = mem_array[7];
		end else if(instruction[25:21] === 5'b01000) begin
			rs_content = mem_array[8];
		end else if(instruction[25:21] === 5'b01001) begin
			rs_content = mem_array[9];
		end else if(instruction[25:21] === 5'b01010) begin
			rs_content = mem_array[10];
		end else if(instruction[25:21] === 5'b01011) begin
			rs_content = mem_array[11];
		end else if(instruction[25:21] === 5'b01100) begin
			rs_content = mem_array[12];
		end else if(instruction[25:21] === 5'b01101) begin
			rs_content = mem_array[13];
		end else if(instruction[25:21] === 5'b01110) begin
			rs_content = mem_array[14];
		end else if(instruction[25:21] === 5'b01111) begin
			rs_content = mem_array[15];
		end else if(instruction[25:21] === 5'b10000) begin
			rs_content = mem_array[16];
		end else if(instruction[25:21] === 5'b10001) begin
			rs_content = mem_array[17];
		end else if(instruction[25:21] === 5'b10010) begin
			rs_content = mem_array[18];
		end else if(instruction[25:21] === 5'b10011) begin
			rs_content = mem_array[19];
		end else if(instruction[25:21] === 5'b10100) begin
			rs_content = mem_array[20];
		end else if(instruction[25:21] === 5'b10101) begin
			rs_content = mem_array[21];
		end else if(instruction[25:21] === 5'b10110) begin
			rs_content = mem_array[22];
		end else if(instruction[25:21] === 5'b10111) begin
			rs_content = mem_array[23];
		end else if(instruction[25:21] === 5'b11000) begin
			rs_content = mem_array[24];
		end else if(instruction[25:21] === 5'b11001) begin
			rs_content = mem_array[25];
		end else if(instruction[25:21] === 5'b11010) begin
			rs_content = mem_array[26];
		end else if(instruction[25:21] === 5'b11011) begin
			rs_content = mem_array[27];
		end else if(instruction[25:21] === 5'b11100) begin
			rs_content = mem_array[28];
		end else if(instruction[25:21] === 5'b11101) begin
			rs_content = mem_array[29];
		end else if(instruction[25:21] === 5'b11110) begin
			rs_content = mem_array[30];
		end else if(instruction[25:21] === 5'b11111) begin
			rs_content = mem_array[31];
		end 
		
		if(instruction[20:16] === 5'b00000)begin
			//rt_content = mem_array[0];
			//you can't write here
		end else if(instruction[20:16] === 5'b00001) begin
			rt_content = mem_array[1];
		end else if(instruction[20:16] === 5'b00010) begin
			rt_content = mem_array[2];
		end else if(instruction[20:16] === 5'b00011) begin
			rt_content = mem_array[3];
		end else if(instruction[20:16] === 5'b00100) begin
			rt_content = mem_array[4];
		end else if(instruction[20:16] === 5'b00101) begin
			rt_content = mem_array[5];
		end else if(instruction[20:16] === 5'b00110) begin
			rt_content = mem_array[6];
		end else if(instruction[20:16] === 5'b00111) begin
			rt_content = mem_array[7];
		end else if(instruction[20:16] === 5'b01000) begin
			rt_content = mem_array[8];
		end else if(instruction[20:16] === 5'b01001) begin
			rt_content = mem_array[9];
		end else if(instruction[20:16] === 5'b01010) begin
			rt_content = mem_array[10];
		end else if(instruction[20:16] === 5'b01011) begin
			rt_content = mem_array[11];
		end else if(instruction[20:16] === 5'b01100) begin
			rt_content = mem_array[12];
		end else if(instruction[20:16] === 5'b01101) begin
			rt_content = mem_array[13];
		end else if(instruction[20:16] === 5'b01110) begin
			rt_content = mem_array[14];
		end else if(instruction[20:16] === 5'b01111) begin
			rt_content = mem_array[15];
		end else if(instruction[20:16] === 5'b10000) begin
			rt_content = mem_array[16];
		end else if(instruction[20:16] === 5'b10001) begin
			rt_content = mem_array[17];
		end else if(instruction[20:16] === 5'b10010) begin
			rt_content = mem_array[18];
		end else if(instruction[20:16] === 5'b10011) begin
			rt_content = mem_array[19];
		end else if(instruction[20:16] === 5'b10100) begin
			rt_content = mem_array[20];
		end else if(instruction[20:16] === 5'b10101) begin
			rt_content = mem_array[21];
		end else if(instruction[20:16] === 5'b10110) begin
			rt_content = mem_array[22];
		end else if(instruction[20:16] === 5'b10111) begin
			rt_content = mem_array[23];
		end else if(instruction[20:16] === 5'b11000) begin
			rt_content = mem_array[24];
		end else if(instruction[20:16] === 5'b11001) begin
			rt_content = mem_array[25];
		end else if(instruction[20:16] === 5'b11010) begin
			rt_content = mem_array[26];
		end else if(instruction[20:16] === 5'b11011) begin
			rt_content = mem_array[27];
		end else if(instruction[20:16] === 5'b11100) begin
			rt_content = mem_array[28];
		end else if(instruction[20:16] === 5'b11101) begin
			rt_content = mem_array[29];
		end else if(instruction[20:16] === 5'b11110) begin
			rt_content = mem_array[30];
		end else if(instruction[20:16] === 5'b11111) begin
			rt_content = mem_array[31];
		end 
	end

	mips_core_testbench tester(result,instruction, rs_content, rt_content);
	//cozemedigim bir nedenden dolayı result haricindeki tum degiskenler ikiser basiyor
	always @(result or rs_content or rt_content) begin
		$display("rs_content: %b",rs_content);
		if(instruction[31:26] === `R_TYPE)
			$display("rt_content: %b",rt_content);	

		$monitor("    result: %b",result);
	end

	always @(result) begin
		if(instruction[31:26] === 6'b000000)begin
			if(instruction[15:11] !== 6'b000000 || instruction[15:11] !== 6'b000001
				|| instruction[15:11] !== 6'b011010 || instruction[15:11] !== 6'b011011)
				mem_array[instruction[15:11]] = result;
		end else
			mem_array[instruction[20:16]] = result;		
		$writememh("registers.h", mem_array, 0, 31);
	end
		
endmodule