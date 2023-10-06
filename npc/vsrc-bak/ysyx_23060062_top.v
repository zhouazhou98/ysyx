module ysyx_23060062_top (
	input clk,

	// 通用寄存器 x0 ~ x31
	input wire [4:0] regs,
	input wire reg_read,
	input wire reg_write,
	input wire [31:0] reg_write,
	output wire [31:0] reg_read,

	// pc
	input wire [31:0] pc_write,
	output wire [31:0] pc_read,
	input wire [31:0] inst_fetch,

	// 内存读写：读/写 pc 内存地址，从寄存器读出 或 向寄存器写入
	input wire [31:0] mem_data_read,
	output wire mem_read,
	output wire mem_write,
	output wire [31:0] mem_data_write

);

// TODO: 时钟呢？什么时候需要时钟？ 
// 	1. 取指令的时候需要时钟？ 没有时钟 inst 寄存器会随着与 内存 的数据总线
// 	的变化而改变，不能这样
// 		取指令前需要将 内存读 信号，内存地址 信号提前给到 控制总线
// 		和 地址总线，这样等 时钟 到来时可以直接从内存中读出数据
//	取指令 -- 读取内存 --> MM 与 Reg 交换
//
//	2. 写入 MM：MM 需要知道处理器将 写 控制信号 和 PC 同时给到 并且在时钟
//	到来时 将数据写入，那如果数据还没有给到呢？可能会向 MM 中写入错误的值
//	因此需要：给数据， 给 PC， 给信号， 等待时钟信号




// 0. 定义寄存器
// TODO:
// 	按理说寄存器这样写不太对吧，尤其是对于 x0 来说，其变成了非固定的
// 	不过这里先暂时这样了 - 为了尽快完成构思
reg [31:0] pc;

// 用来存放当前正在译码的指令
reg [31:0] inst;

// TODO: 这里还是用子文件来实现通用寄存器比较好，给个写入读出接口即可
// 	其实这里可以直接给 rd，不用在指令判断 type 时译码了，还简单省事
ysyx_23060062_regs regs (
	input wire [4:0] regs,
	input wire reg_read,
	input wire reg_write,
	input wire [31:0] reg_write,
	output wire [31:0] reg_read
);

// 1. 取指令
ysyx_23060062_inst_fetch ifetch (
	.pc_in (pc),			// 用来指示从 mem 中哪个地址读取数据
	.mem_read(mem_read),		// 用来更改 读内存 信号
					// 控制信号：用来输出给内存（TODO:这里其实是告诉 C 语言，不过逻辑正确与否先不关系）
	.inst_fetch (inst_fetch)	// 用来输入内存地址 R[pc] 中存储的指令
);

// TODO: 这里到底用 = 还是 <=，如果将 inst 改为 wire 类型显然是不合适的，内存
// 数据总线上不可能一直保持数据，那这样应该必须在 always (@posedge clk) 中了吧
always (@posedge clk) begin
	inst <= inst_fetch;			// 用来将读取到的指令存储到指令寄存器中
end

// 2. 译码 opcode[1:0] --> opcode[4:2] --> opcode[6:5] --> funct3 ? --> funct7 ?
// 		opcode [1:0] : 16 bit ?
// 		opcode [4:2] : 32 bit ?
// 		opcode [6:5] : 对于 32 bit 指令确定 type (I, R, U, S, B, J)
// 	TODO: 这里对 opcode[6:2] 译码可以使用 桶形移位器 
ysyx_23060062_inst_decoder idecode (
	.inst_16bit (inst[1:0]),
	.inst_32bit (inst[4:2]),
	.inst_base_opcode (inst[6:5]),
	.opcode_en (opcode_en[31:0]),
	.opcode_n_bit (opcode_n_bit_en[1:0])
);



// 3.1	type ? --> type decode
ysyx_23060062_inst_type_decode type_decode (
	.opcode_en (opcode_en[31:0]),
	.opcode_n_bit (opcode_n_bit_en[2:0]),
	.inst (inst[31:0]),
	.src1 (src1[31:0]),
	.src2 (src2[31:0]),
	.imm (imm[31:0]),
	.pc_read (pc[31:0])
);
// 3.2 	funct3 ? --> funct7 ? 
// 	TODO: type 的译码需要再做个译码部件来提高效率吗？如果使用 32 位译码器
// 	对 opcode 译码后再进行 type 选择，会不会效率不高？还是说译码部件译码同
// 	时，所有的 type 都在对指令进行类型译码操作，最后根据译码器 32 位输出在
// 	 6 种类型中选择？
// 可以再接个 100 根左右的线，作为指令的使能端，输出
// alu
ysyx_23060062_inst_inst_exec_decode inst_exec_decode (
	.inst (inst[31:0]),
	.memory_op (memory_op),
	.opcode_n_bit (opcode_n_bit_en[1:0]),
	.opcode_en (opcode_en[31:0]),
	.src1 (src1[31:0]),
	.src2 (src2[31:0]),
	.imm (imm[31:0]),
	.pc_read (pc_read)
	.rd_output (rd_data[31:0])
);



// 4. 执行 具体到各个指令实现
// 	TODO: 各个指令执行过程中是否涉及到内存，如何保证 C 代码 与 verilog 实
// 	现的电路之间的 时钟？


// 5. 写回 TODO: 如果指令写回到 pc，能不能保证正确
// 	写回操作需要分组讨论吗？是写到 pc 还是写道 x0~x31





endmodule
