module proje01(r[1:0],a[4:0],b[4:0]);
	input [4:0] a,b;
	output [1:0] r;
	wire zeroControl;
	wire zeroNumber;
	wire AisBiggerThanB;
	wire AisSmallerThanB;
	wire AequalToB;
	wire andOperations;
	
	bigBinaryNumberFound bigNumber(AisBiggerThanB,a,b);
	smallBinaryNumberFound smallNumber(AisSmallerThanB,a,b);
	equalBinaryNumber equalNumber(AequalToB,a,b);
	and 
		n1 (zeroControl,zeroNumber,AequalToB);
	
	or
		n2 (r[0],AisSmallerThanB,zeroControl);
		n3 (r[1],AisBiggerThanB,zeroControl);
	
endmodule