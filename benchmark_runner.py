import subprocess
import csv
import sys
import argparse
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
    parser = argparse.ArgumentParser(
        description='Run SAT solver benchmarks and save results to CSV.'
    )
    
    parser.add_argument('benchmark_dir', help='Path to benchmark directory')
    
    parser.add_argument(
        'solver_path',
        nargs='?',
        default='build/solver',
        help='Path to solver binary (default: build/solver)'
    )
    
    parser.add_argument(
        '-o',
        '--output',
        dest='output_file',
        help='Output CSV filename (saved under benchmarks/results)'
    )

    args = parser.parse_args()
    results_dir = Path.cwd() / 'benchmarks' / 'results'
    results_dir.mkdir(parents=True, exist_ok=True)

    benchmark_dir = Path(args.benchmark_dir)
    solver_path = Path(args.solver_path)
    output_file = results_dir / args.output_file

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
