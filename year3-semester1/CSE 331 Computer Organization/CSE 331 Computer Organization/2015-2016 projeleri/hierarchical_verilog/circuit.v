module circuit (x, y, a, b, c);
	//         ___
	//   x ---|   |--- a = xy
	//        |   |--- b = x + y'
	//   y ---|___|--- c = xy + xy' + x'y
	//
	input x, y;
	output a, b, c;
	wire ny, nx, p1, p2, p3;
	
	and (a, x, y);
	
	not(ny, y);
	or (b, x, ny);
	
	not (nx, x);
	and (p1, x, y);
	and (p2, x, ny);
	and (p3, nx, y);
	or (c, p1, p2, p3);
endmodule