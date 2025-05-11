import subprocess
import multiprocessing
import json
import os
import random
import argparse

class Config:
    EXECUTABLE = './a.out'
    TESTS_FILEPATH = "tests/"
    NUM_WORKERS = multiprocessing.cpu_count()
    OUTPUT_DIR = "output"
    VALGRIND_CMD = "valgrind"
    VALGRIND_ARGS = ["--leak-check=full", "--show-leak-kinds=all", 
                     "--errors-for-leak-kinds=all", "--error-exitcode=1"]

def run_code(input_string, use_valgrind=False):
    """Run a single test with or without valgrind."""
    try:
        if use_valgrind:
            cmd = [Config.VALGRIND_CMD] + Config.VALGRIND_ARGS + [Config.EXECUTABLE]
            process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            stdout, stderr = process.communicate(input=input_string.encode('utf-8'))
            return {
                'stdout': stdout.decode('utf-8') if stdout else '',
                'stderr': stderr.decode('utf-8') if stderr else '',
                'returncode': process.returncode
            }
        else:
            cmd = [Config.EXECUTABLE]
            process = subprocess.Popen(cmd, 
                                    stdin=subprocess.PIPE, 
                                    stdout=subprocess.PIPE, 
                                    stderr=subprocess.PIPE, 
                                    text=True)
            stdout, stderr = process.communicate(input=input_string)
            return {
                'stdout': stdout if stdout else '',
                'stderr': stderr if stderr else '',
                'returncode': process.returncode
            }
    except Exception as e:
        print(f"Error running code: {str(e)}")
        return {'stdout': '', 'stderr': str(e), 'returncode': 1}

def normalize_output(output):
    """Normalize output for comparison."""
    if not output:
        return ""
    
    lines = output.strip().split('\n')
    normalized_lines = []
    
    for line in lines:
        line = line.strip()
        if '_' in line:  # Handle PRINT command output
            classifications = line.strip('_').split('_')
            classifications = [c for c in classifications if c]  # Remove empty strings
            if classifications:
                classifications.sort()
                normalized_lines.append('_'.join(classifications) + '_')
            else:
                normalized_lines.append(line)
        else:
            normalized_lines.append(line)
    
    return '\n'.join(normalized_lines)

def compare_output(correct_solution, current_solution):
    """Compare output after normalizing."""
    normalized_correct = normalize_output(correct_solution)
    normalized_current = normalize_output(current_solution)
    return normalized_correct == normalized_current

def worker(args):
    """Worker process to run tests."""
    worker_id, test_files, valgrind_rate = args
    errors = []
    correct = 0
    total = 0
    
    for test_file in test_files:
        test_path = os.path.join(Config.TESTS_FILEPATH, test_file)
        if not test_file.endswith('.in'):
            continue
            
        # Read the input file
        try:
            with open(test_path, 'r') as f:
                input_string = f.read()
        except Exception as e:
            print(f"Error reading test file {test_path}: {str(e)}")
            continue

        # Read the reference output file
        out_path = test_path.replace('.in', '.out')
        try:
            with open(out_path, 'r') as f:
                reference_solution = f.read()
        except FileNotFoundError:
            print(f"Error: Reference output file not found for {test_file}")
            print("Please generate reference outputs first.")
            continue

        # Run the current test
        run_valgrind = random.random() < valgrind_rate if valgrind_rate < 1.0 else True
        result = run_code(input_string, use_valgrind=run_valgrind)
        current_solution = result['stdout']

        # Compare outputs
        total += 1
        output_correct = compare_output(reference_solution, current_solution)
        memory_safe = result['returncode'] == 0 if run_valgrind else True

        if output_correct and memory_safe:
            correct += 1
        else:
            errors.append({
                "test_file": test_file,
                "input": input_string,
                "reference": reference_solution,
                "current": current_solution,
                "output_correct": output_correct,
                "memory_safe": memory_safe,
                "valgrind_output": result['stderr'] if run_valgrind else "Valgrind not run"
            })

        if total % 10 == 0:
            print(f"Worker {worker_id}: Processed {total} tests")

    return {"worker_id": worker_id, "total": total, "correct": correct, "errors": errors}

def main(valgrind_rate):
    """Main function to coordinate test running."""
    if not os.path.exists(Config.EXECUTABLE):
        print(f"Error: {Config.EXECUTABLE} not found. Please compile the program first.")
        return

    if not os.path.exists(Config.TESTS_FILEPATH):
        print(f"Error: Test directory {Config.TESTS_FILEPATH} not found.")
        return

    os.makedirs(Config.OUTPUT_DIR, exist_ok=True)

    # Get all test files
    test_files = sorted([f for f in os.listdir(Config.TESTS_FILEPATH) if f.endswith('.in')])
    if not test_files:
        print("No test files found.")
        return

    # Check if corresponding .out files exist
    missing_outputs = [f for f in test_files if not os.path.exists(os.path.join(Config.TESTS_FILEPATH, f.replace('.in', '.out')))]
    if missing_outputs:
        print("Error: Missing reference output files for the following tests:")
        for f in missing_outputs:
            print(f"  {f}")
        print("\nPlease ensure all reference output files are present.")
        return

    # Split tests among workers
    chunk_size = len(test_files) // Config.NUM_WORKERS + 1
    worker_args = []
    for i in range(Config.NUM_WORKERS):
        start_idx = i * chunk_size
        end_idx = min(start_idx + chunk_size, len(test_files))
        worker_args.append((i, test_files[start_idx:end_idx], valgrind_rate))

    print(f"Running {len(test_files)} tests with {Config.NUM_WORKERS} workers")
    print(f"Valgrind rate: {valgrind_rate:.1%}")

    # Run tests in parallel
    with multiprocessing.Pool(processes=Config.NUM_WORKERS) as pool:
        results = pool.map(worker, worker_args)

    # Aggregate results
    total = sum(r["total"] for r in results)
    correct = sum(r["correct"] for r in results)
    all_errors = []
    for r in results:
        all_errors.extend(r["errors"])

    # Print errors if any
    if all_errors:
        print("\nFound errors in the following tests:")
        for error in all_errors:
            print(f"\nTest {error['test_file']}:")
            print("Input:")
            print(error['input'])
            if not error["output_correct"]:
                print("Output mismatch:")
                print("Expected output:")
                print(error["reference"])
                print("Your output:")
                print(error["current"])
            if not error["memory_safe"]:
                print("Memory issues detected:")
                print(error["valgrind_output"])

    # Print summary
    print(f"\nTest Summary:")
    print(f"Total Tests: {total}")
    print(f"Passed Tests: {correct}")
    if total > 0:
        print(f"Success Rate: {(correct/total)*100:.2f}%")

    # Save results
    results_data = {
        "total": total,
        "correct": correct,
        "errors": all_errors,
        "valgrind_rate": valgrind_rate
    }
    with open("test_results.json", "w") as f:
        json.dump(results_data, f, indent=2)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Run tests with optional Valgrind memory checking")
    parser.add_argument("--valgrind", nargs='?', const=1.0, type=float, 
                       help="Run Valgrind on tests. If no value provided, run on all tests. "
                            "If value between 0 and 1 provided, run on that fraction of tests.")
    args = parser.parse_args()

    valgrind_rate = args.valgrind if args.valgrind is not None else 0.0
    if valgrind_rate < 0 or valgrind_rate > 1:
        parser.error("Valgrind rate must be between 0 and 1")

    main(valgrind_rate)