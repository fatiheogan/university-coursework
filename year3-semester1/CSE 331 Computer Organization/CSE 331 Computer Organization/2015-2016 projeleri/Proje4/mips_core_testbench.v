`define R_TYPE 000000

`define ADD 6'b100000
`define SUB 6'b100010
`define AND 6'b100100
`define OR 6'b100101
`define SRA 6'b000011
`define SRL 6'b000010
`define SLL 6'b000000
`define SLTU 6'b101011
`define XOR 6'b100110

`define ADDI 6'b001000
`define ADDIU 6'b001001
`define ANDI 6'b001100
`define ORI 6'b001101
`define SLTI 6'b001010
`define LUI 6'b001111
`define XORI 6'b001110

module mips_core_testbench (result, input_instruction, rs_content, rt_content);
	output reg[31:0] result;
	input [31:0] input_instruction;
	input [31:0] rs_content;
	input [31:0] rt_content;
	reg [31:0] sign_extend;
	
	always @(input_instruction or rs_content or rt_content) begin
		if(input_instruction[31:26] === `R_TYPE) begin
			if(input_instruction[5:0] === `ADD) begin 
				$display("add");
				result = rs_content + rt_content;
			end 
			else if(input_instruction[5:0] === `SUB) begin
				$display("sub");
				if(rs_content < rt_content)
					result = rt_content - rs_content;
				else
					result = rs_content - rt_content;
			end
			else if(input_instruction[5:0] === `AND) begin
				$display("and");
				result = rt_content & rs_content;
			end
			else if(input_instruction[5:0] === `OR) begin
				$display("or");
				result = rt_content | rs_content;
			end
			else if(input_instruction[5:0] === `SRA) begin
				$display("sra");
				result = $signed(rt_content) >>> input_instruction[10:6];
			end
			else if(input_instruction[5:0] === `SRL) begin
				$display("srl");
				result = rt_content >> input_instruction[10:6];
			end
			else if(input_instruction[5:0] === `SLL) begin
				$display("sll");
				result = rt_content << input_instruction;
			end
			else if(input_instruction[5:0] === `SLTU) begin
				$display("sltu");
				if($unsigned(rt_content) > $unsigned(rs_content))
					result = 32'b00000000000000000000000000000001;
				else 
					result = 32'b00000000000000000000000000000000;
			end
			else if(input_instruction[5:0] === `XOR)begin
				result = rs_content ^ rt_content;
			end
			else begin 
				$monitor("There is no instruction.1");
			end
		end
		else begin
			if(input_instruction[15:15] === 1'b1)
				sign_extend = {16'b1111111111111111,input_instruction[15:0]};
			else
				sign_extend = {16'b0000000000000000,input_instruction[15:0]};
				
			if (input_instruction[31:26] === `ADDI) begin
				$display("addi");
				result = rs_content + sign_extend;
			end
			else if (input_instruction[31:26] === `ADDIU) begin
				$display("addiu");
				result = $unsigned(rs_content) + sign_extend;
			end
			else if (input_instruction[31:26] === `ORI) begin
				$display("ori");
				result = rs_content | sign_extend;
			end 
			else if (input_instruction[31:26] === `ANDI) begin
				$display("andi");
				result = rs_content & sign_extend;
			end
			else if (input_instruction[31:26] === `SLTI) begin
				$display("slti");
				if(rs_content < sign_extend)
					result = 32'b00000000000000000000000000000001;
				else
					result = 32'b00000000000000000000000000000000; 
			end
			else if (input_instruction[31:26] === `LUI) begin
				$display("lui");
				result = {input_instruction[15:0],16'b0000000000000000};
			end
			else if (input_instruction[31:26] === `XORI) begin
				result = rs_content ^ sign_extend;
			end
			else begin 
				$monitor("There is no instruction.2");
			end
			$display("Sign Extend: %b",sign_extend);
		end
	end
endmodule