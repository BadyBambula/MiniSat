# MiniSat (School Project)

Jednoduchy SAT solver pro formule v DIMACS CNF formatu.

## Pozadavky

- CMake 3.10+
- C++ kompilator s podporou C++11+ (clang nebo g++)
- Python 3 (volitelne, pro benchmark skript)

## Struktura projektu

- `main.cpp` - vstupni bod aplikace
- `cnf/` - parser DIMACS CNF
- `solver/` - implementace SAT solveru
- `benchmarks/` - testovaci CNF instance
- `benchmark_runner.py` - skript pro hromadne mereni

## Build

V koreni projektu spust:

```bash
cmake -S . -B build
cmake --build build
```

Po uspesnem buildu vznikne binarka:

```bash
./build/solver
```

## Spusteni solveru

Zakladni pouziti:

```bash
./build/solver <soubor.cnf>
```

Priklad:

```bash
./build/solver benchmarks/test-sat.cnf
./build/solver benchmarks/test-unsat.cnf
```

Tichy rezim (jen vysledek):

```bash
./build/solver benchmarks/test-sat.cnf -q
```

## Benchmark skript (volitelne)

Skript projde vsechny `.cnf` soubory v zadane slozce, pusti na ne solver a ulozi CSV vysledky.

```bash
python3 benchmark_runner.py <benchmark_dir> [solver_path]
```

Priklady:

```bash
python3 benchmark_runner.py benchmarks/20vars
python3 benchmark_runner.py benchmarks/100vars build/solver
```

Vystupni CSV je ulozeno do:

- `benchmarks/results/<nazev>_results.csv`

kde `<nazev>` je cast cesty za `benchmarks/` (napr. `20vars_results.csv`).

## Poznamka

Pokud build selze kvuli cache nebo starym artefaktum, smaz `build/` a proved build znovu.
