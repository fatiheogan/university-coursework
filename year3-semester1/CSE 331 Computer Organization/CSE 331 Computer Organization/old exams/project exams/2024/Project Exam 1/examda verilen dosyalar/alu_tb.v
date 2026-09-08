module alu_tb;

    // Test inputs
    reg [31:0] a, b;
    reg [3:0] alu_op;
    
    // Test outputs
    wire [31:0] result;
    
    // Instantiate the ALU module
    alu uut (
        .a(a),
        .b(b),
        .alu_op(alu_op),
        .result(result)
    );

    // Test procedure
    initial begin
        // Initialize inputs
        a = 32'h06A0A5A5;   // Test value for 'a'
        b = 32'h0A5A6A5A;   // Test value for 'b'
        alu_op = 4'b0000;   // Add operation (sum)
        
        // Apply test vectors
        #10; // Wait for some time
        a = 32'h0000000F;   // Change 'a'
        b = 32'h00000001;   // Change 'b'
        alu_op = 4'b0001;   // Subtract operation (diff)
        
        #10; // Wait for some time
        a = 32'h000000FF;   // Change 'a'
        b = 32'h0000FFFF;   // Change 'b'
        alu_op = 4'b0010;   // AND operation
        
        #10; // Wait for some time
        a = 32'hFF00FF00;   // Change 'a'
        b = 32'h00FF00FF;   // Change 'b'
        alu_op = 4'b0011;   // OR operation
        
        #10; // Wait for some time
        a = 32'h11111111;   // Change 'a'
        b = 32'h00000000;   // Change 'b'
        alu_op = 4'b0100;   // XOR operation
        
        #10; // Wait for some time
        a = 32'hFFFFFFFF;   // Change 'a'
        b = 32'h12345678;   // Change 'b'
        alu_op = 4'b0101;   // NOR operation
        
        #10; // Wait for some time
        $stop; // Stop the simulation
    end

    // Monitor the results
    initial begin
        $monitor("Time = %0t, a = %h, b = %h, alu_op = %b, result = %h",
                 $time, a, b, alu_op, result);
    end

endmodule