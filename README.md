# MiniSat

SatSolver pro formule v DIMACS CNF formatu.

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

V koreni projektu spustte:

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
./build/solver <soubor.cnf> -q (vrati pouze SAT nebo UNSAT)
```

## Benchmark skript

Skript projde vsechny `.cnf` soubory v zadane slozce, pusti na ne solver a ulozi CSV vysledky.

```bash
python3 benchmark_runner.py <benchmark_dir> [solver_path]
```

Priklady:

```bash
python3 benchmark_runner.py benchmarks/20vars
python3 benchmark_runner.py benchmarks/100vars build/solver (defaultne)
```

Vystupni CSV je ulozeno do:

- `benchmarks/results/<nazev>_results.csv`

kde `<nazev>` je cast cesty za `benchmarks/` (napr. `20vars_results.csv`).
