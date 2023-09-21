
module ysyx_23060062_addi(input wire [31:0] src1, input wire [31:0] imm, output wire [31:0] rd);

// 串行加法器
module ysyx_23060062_adder_1bit ( input a, input b, input cin, output sum, output cout );
	assign sum = a ^ b ^ cin;
	assign cout = a&b | (cin & (a ^ b));
endmodule

`define ysyx_23060062_ADDER ( ADDER_INDEX, INPUT_A, INPUT_B, INPUT_CIN, OUTPUT_SUM, OUTPUT_COUT ) ysyx_23060062_adder_1bit adder_1bit_##INDEX ( .a (INPUT_A), .b (INPUT_B), .cin (INPUT_CIN), .sum (OUTPUT_SUM), .cout (OUTPUT_COUT) );

wire [31:0] c;

`ysyx_23060062_ADDER( 0, a[0], b[0], cin, sum[0], c[0] )

generate
	genvar i;
	for (i = 1; i < 32; i = i + 1) begin
		`ysyx_23060062_ADDER( i, a[i], b[i], c[i-1], sum[i], c[i] )
	end
endgenerate

assign cout = c[31];

endmodule
