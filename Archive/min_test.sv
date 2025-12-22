`timescale 1ns / 1ps

module vector_add #(
	parameter int N = 16
)(
	input logic 	clk,
	input logic 	rst_n,
	input logic [31:0] a[N],
	input logic [31:0] b[N],
	input logic [31:0] c[N]
);

	always_ff @(posedge clk or negedge rst_n) begin

        if (!rst_n) begin

            for (int i = 0; i < N; i++)

                c[i] <= 0;

        end else begin

            for (int i = 0; i < N; i++)

                c[i] <= a[i] + b[i];

        end

    end



endmodule
