#include <stdio.h>	// printf()
#include <stdlib.h>	// rand()
#include <assert.h>	// assert()
#include "verilated.h"	// VerilatedContext
#include "Vtop.h"	// Vtop


int main(int argc, char ** argv) {
	// heap 上分配 VerilatedContex
	VerilatedContext * context_p = new VerilatedContex;
	context_p->commandArgs(argc, argv);

	// heap 上分配 Vtop
	Vtop * top_p = new Vtop{context_p};

	int i = 0;
	while ( i < 100000 ) {
		int a = rand() & 1;
		int b = rand() & 1;

		top_p->a = a;
		top_p->b = b;
		top_p->eval();
		assert(top_p->f == (a^b) );
		context_p->timeInc(1);
	}
	
	return 0;
}
