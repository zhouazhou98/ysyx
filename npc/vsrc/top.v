module top (
	input wire [31:0] src,
	input wire [31:0] imm,
	output wire [31:0] rd,
);

addi m_addi (src, imm, rd);

endmodule
