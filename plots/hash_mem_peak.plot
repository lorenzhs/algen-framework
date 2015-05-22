# IMPORT-DATA memory ../results_memory.txt

set terminal pdf size 13.33cm,10cm linewidth 2.0
set pointsize 0.5
set output "hash_mem_peak.pdf"

set grid xtics ytics

set key bottom left

set title 'Hash table peak memory usage'
set xlabel 'log₂(n)'
set ylabel 'peak memory usage (bytes) / n'

## MULTIPLOT(bench) SELECT log(2, config) as x, avg(peakmem)*1.0/config AS y, MULTIPLOT
## FROM memory GROUP BY bench,MULTIPLOT,x ORDER BY MULTIPLOT,x
plot \
    'hash_mem_peak-data.txt' index 0 title "bench=access" with linespoints, \
    'hash_mem_peak-data.txt' index 1 title "bench=find" with linespoints, \
    'hash_mem_peak-data.txt' index 2 title "bench=ins-del-cycle" with linespoints, \
    'hash_mem_peak-data.txt' index 3 title "bench=insert" with linespoints, \
    'hash_mem_peak-data.txt' index 4 title "bench=insert-find" with linespoints