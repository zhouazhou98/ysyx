# sim_main.cpp
IN_CPP=main.cpp
IN_CPP_DIR=csrc

# verilog input 
IN_V=top
IN_V_DIR=vsrc

# param
PARAM=-CFLAGS -ggdb --cc --exe --build -j 0 -Wall

# GTKWave
GTK_PARAM=--prof-exec


all: sim
	verilator $(GTK_PARAM) $(PARAM) $(IN_CPP_DIR)/$(IN_CPP) $(IN_V_DIR)/$(IN_V).v

run:
	./obj_dir/V$(IN_V)

vcd:
	verilator_gantt ./profile_exec.dat

wave:
	gtkwave ./profile_exec.vcd

sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."

include ../Makefile
