module ysyx_23060062_top (
	// 通用寄存器 x0 ~ x31
	input wire [4:0] regs,
	input wire reg_read,
	input wire reg_write,
	input wire [31:0] reg_read,
	output wire [31:0] reg_write,

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
// 	取指令的时候需要时钟？


// 0. 定义寄存器
// TODO:
// 	按理说寄存器这样写不太对吧，尤其是对于 x0 来说，其变成了非固定的
// 	不过这里先暂时这样了 - 为了尽快完成构思
reg [31:0] pc;

// TODO: 这里还是用子文件来实现通用寄存器比较好，给个写入读出接口即可
// 	其实这里可以直接给 rd，不用在指令判断 type 时译码了，还简单省事
reg [31:0] x0;
reg [31:0] x1;
reg [31:0] x2;
reg [31:0] x3;
reg [31:0] x4;
reg [31:0] x5;
reg [31:0] x6;
reg [31:0] x7;
reg [31:0] x8;
reg [31:0] x9;
reg [31:0] x10;
reg [31:0] x11;
reg [31:0] x12;
reg [31:0] x13;
reg [31:0] x14;
reg [31:0] x15;
reg [31:0] x16;
reg [31:0] x17;
reg [31:0] x18;
reg [31:0] x19;
reg [31:0] x20;
reg [31:0] x21;
reg [31:0] x22;
reg [31:0] x23;
reg [31:0] x24;
reg [31:0] x25;
reg [31:0] x26;
reg [31:0] x27;
reg [31:0] x28;
reg [31:0] x29;
reg [31:0] x30;
reg [31:0] x31;

// 用来存放当前正在译码的指令
reg [31:0] inst;

// 1. 取指令
ysyx_23060062_inst_fetch ifetch (
	.pc_in (pc),			// 用来指示从 mem 中哪个地址读取数据
	.mem_read(mem_read),		// 用来更改 读内存 信号
					// 控制信号：用来输出给内存（TODO:这里其实是告诉 C 语言，不过逻辑正确与否先不关系）
	.inst_fetch (inst_fetch),	// 用来输入内存地址 R[pc] 中存储的指令
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




// 3.1 	funct3 ? --> funct7 ? 
// 	TODO: type 的译码需要再做个译码部件来提高效率吗？如果使用 32 位译码器
// 	对 opcode 译码后再进行 type 选择，会不会效率不高？还是说译码部件译码同
// 	时，所有的 type 都在对指令进行类型译码操作，最后根据译码器 32 位输出在
// 	 6 种类型中选择？
// 3.2	type ? --> type decode
// //////////3.3	exec ? --> 直接到具体指令 ----------- 和 4 重复了吧？



// 4. 执行 具体到各个指令实现
// 	TODO: 各个指令执行过程中是否涉及到内存，如何保证 C 代码 与 verilog 实
// 	现的电路之间的 时钟？


// 5. 写回 TODO: 如果指令写回到 pc，能不能保证正确





endmodule
