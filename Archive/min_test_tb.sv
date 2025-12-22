`timescale 1ns / 1ps



module vector_add_tb;



    parameter int N = 16;



    logic clk;

    logic rst_n;

    logic [31:0] a[N];

    logic [31:0] b[N];

    logic [31:0] c[N];



    // Instantiate DUT

    vector_add #(N) dut (

        .clk(clk),

        .rst_n(rst_n),

        .a(a),

        .b(b),

        .c(c)

    );



    // Clock

    initial clk = 0;

    always #5 clk = ~clk; // 100MHz



    // Test

    initial begin

        rst_n = 0;

        #10 rst_n = 1;



        // Initialize inputs

        for (int i = 0; i < N; i++) begin

            a[i] = i;

            b[i] = N-i;

        end



        #10; // wait one cycle



        // Display results

        for (int i = 0; i < N; i++)

            $display("c[%0d] = %0d", i, c[i]);



        $finish;

    end



endmodule


