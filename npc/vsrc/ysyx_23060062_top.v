module ysyx_23060062_top (
	input wire [31:0] src,
	input wire [31:0] imm,
	output wire [31:0] rd
);

ysyx_23060062_addi\ysyx_23060062_addi m_addi ( 
	.src (src), 
	.imm (imm), 
	.rd (rd) 
);

endmodule
