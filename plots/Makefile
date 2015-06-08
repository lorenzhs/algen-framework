all: generate plot

generate:
	./generate.py

plot:
	for f in *.plot; do\
	    sp-process $$f; \
	    gnuplot $$f; \
	done;

clean:
	rm -rf *.svg *-data.txt *.svg

distclean: clean
	rm -rf *.plot
