// 问题：虽说好像解决了问题，但是还有 16 bit 的指令，可能还需要将 alu 拆解
// 	RV32C 16 bit 指令是根据 opcode[1:0] 判断的，当 opcode[1:0] 不是 2'b11
// 	时，变为 RV32，见 P112

module ysyx_23060062_rv32_alu ( input wire [31:0] en, input wire [2:0] type, input [7:0] funct3_en, input wire [6:0] funct7, input wire [31:0] src1, input wire [31:0] src2, output wire [31:0] rd );

// 1. 根据 type 决定是否译码 funct3 （mux 38）


// 2. 根据 type 决定 funct7 是否其作用


// 3. 根据 5 bit opcode 译码后得到的 32 bit en 使能端判断该操作为 哪种 opcode，
// 	不让所有操作无脑进行（应该会耗电吧，也不懂）


// 4. 具体运算符进行运算

endmodule
