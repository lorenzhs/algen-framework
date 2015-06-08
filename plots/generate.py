#!/usr/bin/env python3
# encoding: utf-8

import os
import argparse

gnuplot_file = """# IMPORT-DATA results {data_fn}

# set terminal pdf size 13.33cm,10cm linewidth 2.0
set terminal svg enhanced size 1024,768 fname 'Verdana' fsize 10 mouse jsdir "."
set output "{out_fn}.svg"

set pointsize 0.5
set grid xtics ytics
set offsets 0,0,graph 0.01,graph 0.01

set key {keypos}

set title '{ds_name} {bench_name}, {plot_desc}'
set xlabel 'log₂(n)'
set ylabel '{plot_desc} per element (n){add}'

{logscale}

## MULTIPLOT(ds) SELECT log(2, config_1) as x, avg({col_name})*1.0/(config_1*{factor}) AS y, MULTIPLOT
## FROM results WHERE bench="{bench_col}" GROUP BY MULTIPLOT,x ORDER BY MULTIPLOT,x
"""

plot_types = [
    ("timer", [("running time", "time", "time", {"factor": 1e6, "add": "in nanoseconds"})]),
    ("PAPI_cache", [("L1 data cache misses", "L1miss", "L1D_cache_misses", {"keypos": "top left"}),
                    ("L2 data cache misses", "L2miss", "L2D_cache_misses", {}),
                    ("L3 cache misses",      "L3miss", "L3_cache_misses", {})]),
    ("PAPI_instr", [("total cycles", "cycles", "Total_cycles", {}),
                    ("instructions completed", "instructions", "Instr_completed", {}),
                    ("branch mispredictions", "branchmiss", "Cond_br_mspredictd", {})]),
    ("memory", [("number of allocations", "mem_mallocs", "mallocs", {"logscale": False}),
                ("total memory allocated", "mem_total", "totalmem", {"logscale": False}),
                ("peak memory usage", "mem_peak", "peakmem", {"logscale": False})])
]

# Name, Abbrevitation, Benchmarks=[(name, col)]
ds_types = [("Hash Table", "hash", [("insert", "insert"),
                                    ("insert+find", "insert-find"),
                                    ("access", "access"),
                                    ("find", "find"),
                                    ("find random", "find-random"),
                                    ("(ins-del-ins)^n (del-ins-del)^n", "ins-del-cycle"),
                                    ("wordcount", "wordcount")]),
            ("Priority Queue", "pq", [("heapsort random", "heapsort-rand"),
                                      ("heapsort permutation", "heapsort-perm"),
                                      ("(push-pop-push)^n (pop-push-pop)^n", "idi^n-did^n"),
                                      ("pop", "pop"),
                                      ("push", "push"),
                                      ("push-pop-mix on full heap", "push-pop-mix")])]

data_fn_pattern = "{prefix}/results_{ds}_{res}.txt"
out_fn_pattern = "{ds}_{bench}_{plot}"

parser = argparse.ArgumentParser(description='Generate plots with GNUplot')
parser.add_argument('--all', dest='check_existence', action='store_false',
    help='Generate all plots, even if accompanying data file does not exist')
parser.add_argument('--prefix', dest='prefix', default='.',
    metavar='PATH', help='Location of data files')
args = parser.parse_args()

for (ds_name, ds_abbrv, ds_bench) in ds_types:
    for (bench_name, bench_col) in ds_bench:
        for (fn_segment, plots) in plot_types:
            data_fn = data_fn_pattern.format(prefix=args.prefix, ds=ds_abbrv, res=fn_segment)

            # Check whether data file exists
            if args.check_existence and not os.path.isfile(data_fn):
                continue

            for (plot_desc, plot_name, col_name, opts) in plots:
                out_fn_base = out_fn_pattern.format(ds=ds_abbrv, bench=bench_col, plot=plot_name)

                # Parse options
                add = opts["add"] if "add" in opts else ""
                factor = float(opts["factor"]) if "factor" in opts else 1.0
                keypos = opts["keypos"] if "keypos" in opts else "bottom left"

                if "logscale" in opts and opts["logscale"] in [0, False]:
                    logscale = ""
                else:
                    logscale = "set logscale y"

                # Fill in GNUplot file
                plotfile = gnuplot_file.format(data_fn=data_fn, out_fn=out_fn_base,
                    keypos=keypos, ds_name=ds_name, bench_name=bench_name,
                    plot_desc=plot_desc, add=add, logscale=logscale,
                    bench_col=bench_col, col_name=col_name, factor=factor)

                with open(out_fn_base + '.plot', 'w') as outfile:
                    outfile.write(plotfile)
