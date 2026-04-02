import subprocess
import csv
import sys
from pathlib import Path


def parse_solver_output(output):
    """Parse solver output and extract variables, clauses, result, and time."""
    lines = output.strip().split('\n')
    data = {}

    for line in lines:
        line = line.strip()
        if not line:
            continue
        if line.startswith('Variables:'):
            data['variables'] = int(line.split(':')[1].strip())
        elif line.startswith('Clauses:'):
            data['clauses'] = int(line.split(':')[1].strip())
        elif line.startswith('Result:'):
            data['result'] = line.split(':')[1].strip()
        elif line.startswith('Time needed:'):
            parts = line.split(':')[1].strip().split()
            if parts:
                data['time'] = float(parts[0])

    return data


def run_benchmark(solver_path, cnf_file):
    """Run solver on CNF file and return results."""
    try:
        result = subprocess.run(
            [solver_path, cnf_file],
            capture_output=True,
            text=True,
            timeout=30
        )
        output = result.stdout + result.stderr
        parsed = parse_solver_output(output)
        return parsed
    except subprocess.TimeoutExpired:
        return {'error': 'Timeout'}
    except Exception as e:
        return {'error': str(e)}


def main():
    if len(sys.argv) < 2:
        print(
            "Usage: python3 benchmark_runner.py <benchmark_dir> [solver_path]")
        print("Example: python3 benchmark_runner.py benchmarks/20vars build/solver")
        sys.exit(1)

    benchmark_dir = Path(sys.argv[1])
    solver_path = Path(sys.argv[2]) if len(
        sys.argv) > 2 else Path('build/solver')

    benchmark_name = benchmark_dir.name
    parts = benchmark_dir.parts
    if 'benchmarks' in parts:
        benchmark_index = parts.index('benchmarks')
        if benchmark_index + 1 < len(parts):
            benchmark_name = parts[benchmark_index + 1]

    results_dir = Path.cwd() / 'benchmarks' / 'results'
    results_dir.mkdir(parents=True, exist_ok=True)
    output_file = results_dir / f"{benchmark_name}_results.csv"

    if not benchmark_dir.exists() or not benchmark_dir.is_dir():
        print(f"Error: Benchmark directory not found: {benchmark_dir}")
        sys.exit(1)

    # Check if solver exists
    if not solver_path.exists():
        print(f"Error: Solver not found at {solver_path}")
        sys.exit(1)

    # Find all CNF files
    cnf_files = sorted(benchmark_dir.rglob('*.cnf'))

    if not cnf_files:
        print(f"No CNF files found in {benchmark_dir}")
        sys.exit(1)

    print(f"Found {len(cnf_files)} CNF files")
    print(f"Running benchmarks...")

    # Prepare CSV
    fieldnames = ['filename', 'variables', 'clauses', 'result', 'time_seconds']
    results = []

    for i, cnf_file in enumerate(cnf_files, 1):
        relative_path = cnf_file.relative_to('.')
        print(f"[{i}/{len(cnf_files)}] {relative_path}...", end=' ', flush=True)

        parsed = run_benchmark(str(solver_path), str(cnf_file))

        if 'error' in parsed:
            print(f"ERROR: {parsed['error']}")
            continue

        row = {
            'filename': str(relative_path),
            'variables': parsed.get('variables', 'N/A'),
            'clauses': parsed.get('clauses', 'N/A'),
            'result': parsed.get('result', 'N/A'),
            'time_seconds': parsed.get('time', 'N/A')
        }
        results.append(row)
        print(f"OK ({parsed.get('result', 'N/A')}, {parsed.get('time', 0):.6f}s)")

    # Write CSV
    measured_times = [
        row['time_seconds']
        for row in results
        if isinstance(row.get('time_seconds'), (int, float))
    ]
    avg_time = (sum(measured_times) / len(measured_times)
                ) if measured_times else 0.0

    with open(output_file, 'w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(results)
        writer.writerow({
            'filename': 'AVERAGE',
            'variables': '',
            'clauses': '',
            'result': f"{len(measured_times)} samples",
            'time_seconds': f"{avg_time:.6f}"
        })

    print(f"\nResults saved to {output_file}")
    print(f"Total: {len(results)} files processed")


if __name__ == '__main__':
    main()
