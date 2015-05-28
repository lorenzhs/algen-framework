# IMPORT-DATA timer results_timer.txt
# IMPORT-DATA PAPI_cache results_PAPI_cache.txt
# IMPORT-DATA PAPI_instr results_PAPI_instr.txt
# IMPORT-DATA memory results_memory.txt

set terminal pdf size 13.33cm,10cm linewidth 2.0
set pointsize 0.5
set output "hash_time.pdf"

set grid xtics ytics

set key top left

set title 'Hash table plot'
set xlabel 'log₂(n)'
set ylabel 'running time / n in nanoseconds'

set logscale y

## MULTIPLOT(ds,bench) SELECT log(2, config) as x, avg(time)*1.0/config*1000000.0 AS y, MULTIPLOT
## FROM timer GROUP BY bench,MULTIPLOT,x ORDER BY MULTIPLOT,x
plot \
    'hash_time-data.txt' index 0 title "ds=dense-hash-map,bench=access" with linespoints, \
    'hash_time-data.txt' index 1 title "ds=dense-hash-map,bench=find" with linespoints, \
    'hash_time-data.txt' index 2 title "ds=dense-hash-map,bench=ins-del-cycle" with linespoints, \
    'hash_time-data.txt' index 3 title "ds=dense-hash-map,bench=insert" with linespoints, \
    'hash_time-data.txt' index 4 title "ds=dense-hash-map,bench=insert-find" with linespoints, \
    'hash_time-data.txt' index 5 title "ds=sparse-hash-map,bench=access" with linespoints, \
    'hash_time-data.txt' index 6 title "ds=sparse-hash-map,bench=find" with linespoints, \
    'hash_time-data.txt' index 7 title "ds=sparse-hash-map,bench=ins-del-cycle" with linespoints, \
    'hash_time-data.txt' index 8 title "ds=sparse-hash-map,bench=insert" with linespoints, \
    'hash_time-data.txt' index 9 title "ds=sparse-hash-map,bench=insert-find" with linespoints, \
    'hash_time-data.txt' index 10 title "ds=std::unordered-map,bench=access" with linespoints, \
    'hash_time-data.txt' index 11 title "ds=std::unordered-map,bench=find" with linespoints, \
    'hash_time-data.txt' index 12 title "ds=std::unordered-map,bench=ins-del-cycle" with linespoints, \
    'hash_time-data.txt' index 13 title "ds=std::unordered-map,bench=insert" with linespoints, \
    'hash_time-data.txt' index 14 title "ds=std::unordered-map,bench=insert-find" with linespoints
