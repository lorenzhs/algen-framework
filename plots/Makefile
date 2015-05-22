%: %.plot
	sp-process $@.plot
	gnuplot $@.plot

all: hash_time hash_L1miss hash_L2miss hash_L3miss hash_branchmiss \
	hash_cycles hash_instructions hash_mem_peak hash_mem_total hash_mem_mallocs

clean:
	rm -rf *.pdf *-data.txt
