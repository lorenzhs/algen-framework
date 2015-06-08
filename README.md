# Algorithm Engineering Programmieraufgaben

Implementieren und evaluieren Sie effiziente Datenstrukturen in C++11/14. Nutzen Sie dafür das vorgegebene Framework. Das Framework gibt ein festes Interface für die Datenstrukturen vor, das Sie einhalten müssen (siehe `pq/priority_queue.h` und `hashtable/hashtable.h`). Für Beispiele, wie eine Implementierung dieser Interfaces aussehen kann, können Sie sich die Wrapper um `std::unordered_map` (`hashtable/unordered_map.h`) und `std::priority_queue` (`pq/std_pq.h`) ansehen. Die Implementierungen sollen allgemein nutzbar sein, daher sind beispielsweise die Datentypen als Template-Parameter gegeben.

Da es eine Vielzahl verschiedener Möglichkeiten gibt, können (und sollen) beide Aufgaben von mehreren Personen bearbeitet werden. Diese sollen gegen die Referenzimplementierungen und am Ende auch miteinander verglichen werden, wozu das Framework diverse Benchmarks enthält.

## Priority Queues.
Mögliche Varianten:

- Binary und d-ary Heaps
- Fibonacci Heaps
- Pairing Heaps
- Einfache Sequence Heaps ("mittelgroße" Variante aus der Vorlesung)
- Binomial Heaps
- Rank-Pairing Heaps
- Weitere Vorschläge willkommen!

Neben Microbenchmarks, die die Performance der einzelnen Operationen und Abfolgen von Operationen messen, wird auch die Performance in Anwendungen wie Heapsort gemessen.

## Hashtabellen.
Mögliche Varianten:

- Open Addressing mit verschiedenen Probing-Strategien (Linear oder Quadratic Probing, Double Hashing, ...) oder Löschstrategien (z.B. Löschmarkierungen mit Verschieben beim Suchen), etc.
- Hashing mit Chaining. Auch hier gibt es Optionen: Wird das erste Element in der Tabelle gespeichert, oder besteht der Eintrag nur aus einem Listenzeiger? Wie sind die Elemente in der Liste angeordnet (Sortiert? Move-to-Front-Heuristik? Unsortiert in Einfügereihenfolge?), etc.
- Cuckoo Hashing
- Cuckoo Hashing with Pages (Dietzfelbinger, Mitzenmacher, Rink 2011)
- Dynamic Perfect Hashing (im Buch von Mehlhorn und Sanders beschrieben)
- Weitere Vorschläge willkommen!

Hier wird es wieder Microbenchmarks und synthetische Benchmarks geben, die die Performance der Operationen alleine bzw. in bestimmten Kombinationen messen wird. Zusätzlich wird es auch hier Anwendungsbenchmarks geben (z.B. word count).

## Implementierung

Sofern eine Datenstruktur verschiedene Strategien bietet (bspw. Probing- oder Löschstrategien einer Hashtabelle mit Open Addressing), sollten diese über Template-Parameter spezifizierbar sein. Es bietet sich dazu an, Helferklassen zu schreiben, die diese Strategien implementieren (bpsw in `operator()(<Argumente>)`).

Sie sollten alle möglichen Kombinationen von Strategien in einer Funktion

```c++
static void register_contenders(common::contender_list<BASISKLASSE> &list)
```

in der Kandidatenliste `list` mit Factories registrieren. Beispiele dafür finden Sie in den implementierten Wrapperklassen. Diese müssen Sie dann in der Haupt-Datei (`bench_pq.cpp` oder `bench_hash.cpp`) an der mit "// TODO: add your own implementation here!" markierten Stelle registrieren.

Bitte beachten Sie, dass Sie *alle* Funktionen des Interfaces implementieren müssen, da es sich bei den Interfaces um abstrakte Klassen handelt und der Compiler sich sonst beschwert. Die Verwendung des `override`-Keywords wird empfohlen.

Performance-Hinweis: Ja, die Vererbung hat Performance-Overhead. Dieser ist für das Benchmarking leider unvermeidbar. Für einen Produktiveinsatz der Datenstrukturen reicht es aber, die Vererbung (und `register_contenders`) zu entfernen, da nur die Signaturen vererbt werden. Zudem betrifft dieser Overhead alle Implementierungen in gleichem Maße, ist also auf keine Weise "unfair".

## Installation

```bash
git clone https://git.scc.kit.edu/lorenz/algen-framework.git
git submodule init
git submodule update
```

## Verwendung im ATIS-Pool

Da in der ATIS Fedora 20 läuft und kein ausreichend aktueller Compiler gebundlet ist, habe ich einen solchen für alle zugreifbar in meinem Home-Verzeichnis installiert. Alle Einstellungen dazu sind im alternativen Makefile `Makefile.atis` getroffen. Für die Verwendung im ATIS-Pool daher bitte entweder `Makefile.atis` in `Makefile` umbenennen oder `make -f Makefile.atis` verwenden.

Für die Ausführung der Binaries ist es erforderlich, den Library-Suchpfad wie folgt zu modifizieren:

```bash
export LD_LIBRARY_PATH=/home/stud/s_huebsc/algen/gcc-4.9.2/lib64:/home/stud/s_huebsc/algen/lib
```

## Voraussetzungen

Da das Framework Funktionalität aus dem kommenden C++14-Standard verwendet, ist der GNU `g++`-Compiler in Version 4.9 oder neuer, bzw. der `clang++`-Compiler in Version 3.4 oder neuer erforderlich. Achtung: clang 3.4 und 3.5 können das `compare`-Target nicht mit Debuginformationen übersetzen. In diesem Fall bitte das Flag `-g` entfernen oder einen anderen Compiler verwenden.

Folgende Libraries sind erforderlich:

- libPAPI für die Messung von Cache-/Branch-Misses etc
- boost-serialize für die Serialisierung der Ergebnisse für die spätere Analyse
- malloc_count für Speichermessungen (als submodule enthalten)

Als Buildsystem wird GNU make verwendet. Folgende Targets sid vordefiniert:

- `bench_hash` und `bench_pq` führen Zeitmessungen und Performance-Counter-Messungen (mit libpapi) durch.
- `bench_hash_malloc` und `bench_pq_malloc` messen den Speicherverbrauch. Diese sind aus technischen Gründen ein eigenes Binary.
- `debug_{pq,hash}{,_malloc}` tun ebendies ohne Compileroptimierungen für vereinfachtes Debugging
- `sanitize_{pq,hash}` verwenden Address Sanitizer (ASan) [1], um häufige Speicherfehler und Speicherlecks zu finden. Da ASan nicht mit der malloc-Instrumentation kompatibel ist, existieren die entsprechenden `*_malloc`-Targets nicht.
- `compare` erlaubt die nachträgliche Analyse der Ergebnisse
- Die `run_*`-Targets hängen von den Compile-Targets ab und führen diese aus. Nicht besonders notwendig, aber angenehm ;)

Die Binaries enthalten kurze Hilfetexte zur Ausführung (Parameter `-h`).


[1] http://clang.llvm.org/docs/AddressSanitizer.html