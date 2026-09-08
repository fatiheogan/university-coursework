module full_my_adder(a,b,cin,sum);
	
	input [3:0]a;
	input [3:0]b;
	input cin;
	output [3:0]sum;
	
	wire [3:0]carry;
	
	my_adder_base g1(a[0],b[0],cin,sum[0],carry[0]);
	my_adder_base g2(a[1],b[1],carry[0],sum[1],carry[1]);
	my_adder_base g3(a[2],b[2],carry[1],sum[2],carry[2]);
	my_adder_base g4(a[3],b[3],carry[2],sum[3],carry[3]);
	
endmodule
