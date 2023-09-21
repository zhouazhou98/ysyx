module addi (
	input wire [31:0] src,
	input wire [31:0] imm,
	output wire [31:0] rd,
);

assign rd = imm + src;

endmodule
