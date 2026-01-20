#!/usr/bin/env python3
"""
Regression test runner for xasm++

Compares xasm++ output against vasm-ext golden binaries to ensure
byte-for-byte compatibility.

Usage:
    python3 run_regression.py [--verbose] [test-name]
    python3 run_regression.py --all
    python3 run_regression.py scmasm/basic
"""

import argparse
import filecmp
import os
import subprocess
import sys
from pathlib import Path
from typing import List, Tuple

# Colors for output
class Color:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    END = '\033[0m'

def find_test_files(test_dir: Path) -> List[Path]:
    """Find all .asm files in test directory."""
    return sorted(test_dir.glob('**/*.asm'))

def run_xasm(asm_file: Path, output_dir: Path, cpu: str, syntax: str) -> Tuple[bool, str]:
    """
    Run xasm++ on assembly file.

    Returns (success, error_message)
    """
    bin_file = output_dir / f"{asm_file.stem}.bin"

    cmd = [
        "xasm++",
        "--cpu", cpu,
        "--syntax", syntax,
        "--output", str(bin_file),
        str(asm_file)
    ]

    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=30
        )

        if result.returncode != 0:
            return False, f"xasm++ failed: {result.stderr}"

        if not bin_file.exists():
            return False, f"Output file not created: {bin_file}"

        return True, ""

    except FileNotFoundError:
        return False, "xasm++ not found in PATH (not built yet?)"
    except subprocess.TimeoutExpired:
        return False, "xasm++ timeout (> 30s)"
    except Exception as e:
        return False, f"Unexpected error: {str(e)}"

def compare_binaries(actual: Path, expected: Path, verbose: bool = False) -> Tuple[bool, str]:
    """
    Compare two binary files byte-for-byte.

    Returns (match, diff_message)
    """
    if not expected.exists():
        return False, f"Expected file not found: {expected}"

    if not actual.exists():
        return False, f"Actual file not found: {actual}"

    if filecmp.cmp(actual, expected, shallow=False):
        return True, ""

    # Files differ - generate diff message
    with open(expected, 'rb') as f:
        expected_bytes = f.read()

    with open(actual, 'rb') as f:
        actual_bytes = f.read()

    msg = [f"Binary mismatch: {actual.name}"]
    msg.append(f"  Expected size: {len(expected_bytes)} bytes")
    msg.append(f"  Actual size: {len(actual_bytes)} bytes")

    if verbose and len(expected_bytes) == len(actual_bytes):
        # Show first few differences
        diffs = []
        for i, (e, a) in enumerate(zip(expected_bytes, actual_bytes)):
            if e != a and len(diffs) < 10:
                diffs.append(f"  Offset 0x{i:04X}: expected 0x{e:02X}, got 0x{a:02X}")

        if diffs:
            msg.append("  First differences:")
            msg.extend(diffs)

    return False, "\n".join(msg)

def run_test(test_file: Path, test_root: Path, output_dir: Path, verbose: bool = False) -> bool:
    """
    Run a single regression test.

    Returns True if test passed.
    """
    # Determine CPU and syntax from directory structure
    # e.g., regression/scmasm/test.asm -> cpu=6502, syntax=scmasm
    relative_path = test_file.relative_to(test_root)
    syntax = relative_path.parts[0]

    # Map syntax to CPU (default mappings)
    syntax_to_cpu = {
        'scmasm': '6502',
        'merlin': '65816',
        'edtasm': '6809',
        'edtasm_m80': 'z80',
        'motorola': '68000',
        'zilog': 'z80',
        'intel': '8080',
        'm6800': '6800',
    }

    cpu = syntax_to_cpu.get(syntax, '6502')

    print(f"{Color.BLUE}Testing:{Color.END} {relative_path} (cpu={cpu}, syntax={syntax})")

    # Run xasm++
    success, error = run_xasm(test_file, output_dir, cpu, syntax)

    if not success:
        print(f"  {Color.RED}✗ FAIL:{Color.END} {error}")
        return False

    # Compare output
    actual_bin = output_dir / f"{test_file.stem}.bin"
    expected_bin = test_file.parent / "expected" / f"{test_file.stem}.bin"

    match, diff_msg = compare_binaries(actual_bin, expected_bin, verbose)

    if match:
        print(f"  {Color.GREEN}✓ PASS{Color.END}")
        return True
    else:
        print(f"  {Color.RED}✗ FAIL:{Color.END}")
        if verbose:
            print(diff_msg)
        return False

def main():
    parser = argparse.ArgumentParser(description='Run xasm++ regression tests')
    parser.add_argument('test', nargs='?', help='Specific test to run (e.g., scmasm/basic)')
    parser.add_argument('--all', action='store_true', help='Run all tests')
    parser.add_argument('--verbose', '-v', action='store_true', help='Verbose output')

    args = parser.parse_args()

    # Find test root directory
    script_dir = Path(__file__).parent
    test_root = script_dir
    output_dir = test_root / "actual"
    output_dir.mkdir(exist_ok=True)

    # Determine which tests to run
    if args.test:
        # Single test or directory
        test_path = test_root / args.test
        if test_path.is_file():
            test_files = [test_path]
        elif test_path.is_dir():
            test_files = find_test_files(test_path)
        else:
            print(f"{Color.RED}Error:{Color.END} Test not found: {args.test}")
            return 1
    else:
        # All tests
        test_files = find_test_files(test_root)

    if not test_files:
        print(f"{Color.YELLOW}Warning:{Color.END} No test files found")
        return 0

    print(f"\n{Color.BLUE}Running {len(test_files)} regression test(s)...{Color.END}\n")

    # Run tests
    passed = 0
    failed = 0

    for test_file in test_files:
        if 'expected' in test_file.parts:
            continue  # Skip expected output files

        if run_test(test_file, test_root, output_dir, args.verbose):
            passed += 1
        else:
            failed += 1

    # Summary
    print(f"\n{Color.BLUE}{'='*60}{Color.END}")
    print(f"Total: {passed + failed} tests")
    print(f"{Color.GREEN}Passed: {passed}{Color.END}")

    if failed > 0:
        print(f"{Color.RED}Failed: {failed}{Color.END}")
        return 1
    else:
        print(f"\n{Color.GREEN}✓ All regression tests passed!{Color.END}")
        return 0

if __name__ == '__main__':
    sys.exit(main())
