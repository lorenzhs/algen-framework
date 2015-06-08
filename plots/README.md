# Plots

Die von `test_hash` und `test_pq` geschriebenen Ergebnisdateien werden von [SqlPlotTools](https://github.com/bingmann/sqlplot-tools) geparsed, und die SQL-Queries in den Plot-Dateien visualisieren einen Aspekt der Daten.

# Usage

```bash
cp ../results*.txt .
make all
```

tut das Richtige™ und generiert Plots für alle Experimente, für die Daten vorliegen. Falls auch Plots, für die (noch) keine Daten vorliegen, generiert werden sollen, hilft `./generate.py --all`.

# TODO

HTML-Seite, auf der die Plots in Kategorien unterteilt sind
