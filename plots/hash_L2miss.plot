# IMPORT-DATA PAPI_cache results_PAPI_cache.txt

set terminal pdf size 13.33cm,10cm linewidth 2.0
set pointsize 0.5
set output "hash_L2miss.pdf"

set grid xtics ytics

set key bottom left

set title 'Hash table L2 data cache misses'
set xlabel 'log₂(n)'
set ylabel 'L2 data cache misses / n'

## MULTIPLOT(bench) SELECT log(2, config) as x, avg(L2D_cache_misses)*1.0/config AS y, MULTIPLOT
## FROM PAPI_cache GROUP BY bench,MULTIPLOT,x ORDER BY MULTIPLOT,x
plot \
    'hash_L2miss-data.txt' index 0 title "bench=access" with linespoints, \
    'hash_L2miss-data.txt' index 1 title "bench=find" with linespoints, \
    'hash_L2miss-data.txt' index 2 title "bench=ins-del-cycle" with linespoints, \
    'hash_L2miss-data.txt' index 3 title "bench=insert" with linespoints, \
    'hash_L2miss-data.txt' index 4 title "bench=insert-find" with linespoints
