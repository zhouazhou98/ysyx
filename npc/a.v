import "DPI-C" function word_t vaddr_read(input vaddr_t addr, input int len);
import "DPI-C" function void vaddr_write(input vaddr_t addr, input int len, input word_t data);

module top (input wire[31:0] addr, output wire[31:0] len);

always@(*) begin
	// 在实际调用过程中还需要对 len 进行判断，可以使用译码器接使能输入端
	wire [7:0] data = vaddr_read(addr, 1);
end


endmodule
