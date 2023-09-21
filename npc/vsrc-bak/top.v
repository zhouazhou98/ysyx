module top (
	input wire [31:0] src1,
	input wire [31:0] src2,
	input wire [31:0] imm,
	input wire [31:0] pc,
	input wire [6:0] opcode,
	input wire [2:0] funct3,
	input wire [6:0] funct7,
	output wire [31:0] rd,
	output wire [31:0] pc,
);


endmodule
