#!/usr/bin/env python3
"""
A2oSX Build Validation Suite

Tests xasm++ capability to assemble A2oSX operating system source files.
Documents expected behavior and current limitations.

Usage:
    ./validation_suite.py --xasm /path/to/xasm++
    ./validation_suite.py --all    # Run all tests
    ./validation_suite.py --known-good  # Only run tests that should pass
"""

import argparse
import subprocess
import sys
from pathlib import Path
from typing import List, Tuple, Optional
from dataclasses import dataclass
from enum import Enum


class TestStatus(Enum):
    """Test execution status"""
    PASS = "PASS"
    FAIL = "FAIL"
    SKIP = "SKIP"
    BLOCKED = "BLOCKED"  # Blocked by known bugs


@dataclass
class TestCase:
    """Individual validation test case"""
    name: str
    source_file: str
    description: str
    expected_status: TestStatus
    blocking_bugs: List[str] = None
    
    def __post_init__(self):
        if self.blocking_bugs is None:
            self.blocking_bugs = []


@dataclass
class TestResult:
    """Test execution result"""
    test: TestCase
    status: TestStatus
    output: str
    error: str = ""


class A2oSXValidator:
    """Validation suite for A2oSX assembly with xasm++"""
    
    def __init__(self, xasm_path: str):
        self.xasm_path = Path(xasm_path)
        self.test_dir = Path(__file__).parent
        self.results: List[TestResult] = []
        
        # Verify xasm++ exists
        if not self.xasm_path.exists():
            raise FileNotFoundError(f"xasm++ not found: {self.xasm_path}")
    
    def define_tests(self) -> List[TestCase]:
        """Define all validation test cases"""
        return [
            # Level 1: Basic functionality (should work)
            TestCase(
                name="minimal_assembly",
                source_file="test_minimal.S",
                description="Minimal assembly with .OP and .OR",
                expected_status=TestStatus.PASS
            ),
            
            TestCase(
                name="eq_directive",
                source_file="test_eq.S",
                description="Symbol definition with .EQ",
                expected_status=TestStatus.PASS
            ),
            
            # Level 2: Symbol usage (currently blocked)
            TestCase(
                name="symbol_addressing",
                source_file="test_symbols.S",
                description="Using .EQ symbols in addressing modes",
                expected_status=TestStatus.BLOCKED,
                blocking_bugs=["BUG-001: Symbols don't work in addressing modes"]
            ),
            
            # Level 3: Data directives
            TestCase(
                name="ascii_string",
                source_file="test_data_as.S",
                description="ASCII string with .AS directive",
                expected_status=TestStatus.PASS
            ),
            
            TestCase(
                name="hex_data",
                source_file="test_data_hs.S",
                description="Hex data with .HS directive",
                expected_status=TestStatus.PASS
            ),
            
            TestCase(
                name="data_bytes",
                source_file="test_data_da.S",
                description="Data bytes with .DA directive",
                expected_status=TestStatus.PASS
            ),
            
            # Level 4: Block storage (blocked)
            TestCase(
                name="block_storage",
                source_file="test_bs.S",
                description="Reserve space with .BS directive",
                expected_status=TestStatus.BLOCKED,
                blocking_bugs=["BUG-002: .BS interprets as binary, not byte count"]
            ),
            
            # Level 5: Expressions (blocked)
            TestCase(
                name="expression_low_byte",
                source_file="test_expr_lo.S",
                description="Low byte expression #<LABEL",
                expected_status=TestStatus.BLOCKED,
                blocking_bugs=["BUG-003: Expression #<LABEL not supported"]
            ),
            
            TestCase(
                name="expression_high_byte",
                source_file="test_expr_hi.S",
                description="High byte expression #>LABEL",
                expected_status=TestStatus.BLOCKED,
                blocking_bugs=["BUG-003: Expression #>LABEL not supported"]
            ),
            
            TestCase(
                name="expression_offset",
                source_file="test_expr_offset.S",
                description="Offset expression LABEL+N",
                expected_status=TestStatus.BLOCKED,
                blocking_bugs=["BUG-003: Expression LABEL+N not supported"]
            ),
            
            # Level 6: Real A2oSX samples
            TestCase(
                name="a2osx_simple",
                source_file="sample_simple.S",
                description="Simplified A2oSX-style code",
                expected_status=TestStatus.BLOCKED,
                blocking_bugs=["BUG-001", "BUG-002", "BUG-003"]
            ),
        ]
    
    def run_test(self, test: TestCase, verbose: bool = False) -> TestResult:
        """Execute a single test case"""
        source_path = self.test_dir / test.source_file
        output_path = self.test_dir / f"{test.source_file}.bin"
        
        # Skip if source doesn't exist
        if not source_path.exists():
            return TestResult(
                test=test,
                status=TestStatus.SKIP,
                output=f"Source file not found: {source_path}",
                error=""
            )
        
        # Skip blocked tests unless explicitly requested
        if test.expected_status == TestStatus.BLOCKED:
            return TestResult(
                test=test,
                status=TestStatus.BLOCKED,
                output=f"Test blocked by known bugs: {', '.join(test.blocking_bugs)}",
                error=""
            )
        
        # Clean previous output
        if output_path.exists():
            output_path.unlink()
        
        # Run xasm++
        cmd = [
            str(self.xasm_path),
            "--syntax", "scmasm",
            "--cpu", "65c02",
            str(source_path),
            "-o", str(output_path)
        ]
        
        try:
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=10
            )
            
            # Check if assembly succeeded
            if result.returncode == 0 and output_path.exists():
                status = TestStatus.PASS
            else:
                status = TestStatus.FAIL
            
            return TestResult(
                test=test,
                status=status,
                output=result.stdout,
                error=result.stderr if result.returncode != 0 else ""
            )
            
        except subprocess.TimeoutExpired:
            return TestResult(
                test=test,
                status=TestStatus.FAIL,
                output="",
                error="Test timed out after 10 seconds"
            )
        except Exception as e:
            return TestResult(
                test=test,
                status=TestStatus.FAIL,
                output="",
                error=f"Exception: {str(e)}"
            )
    
    def run_all(self, skip_blocked: bool = True, verbose: bool = False):
        """Run all validation tests"""
        tests = self.define_tests()
        
        print(f"A2oSX Build Validation Suite")
        print(f"xasm++: {self.xasm_path}")
        print(f"Tests: {len(tests)}")
        print("=" * 80)
        print()
        
        for test in tests:
            if skip_blocked and test.expected_status == TestStatus.BLOCKED:
                print(f"[SKIP] {test.name}: {test.description}")
                if verbose:
                    print(f"       Blocking bugs: {', '.join(test.blocking_bugs)}")
                continue
            
            result = self.run_test(test, verbose)
            self.results.append(result)
            
            # Print result
            status_str = result.status.value
            if result.status == TestStatus.PASS:
                status_str = f"\033[32m{status_str}\033[0m"  # Green
            elif result.status == TestStatus.FAIL:
                status_str = f"\033[31m{status_str}\033[0m"  # Red
            elif result.status == TestStatus.BLOCKED:
                status_str = f"\033[33m{status_str}\033[0m"  # Yellow
            
            print(f"[{status_str}] {test.name}: {test.description}")
            
            if verbose and result.error:
                print(f"       Error: {result.error[:200]}")
        
        print()
        self.print_summary()
    
    def print_summary(self):
        """Print test summary"""
        total = len(self.results)
        passed = sum(1 for r in self.results if r.status == TestStatus.PASS)
        failed = sum(1 for r in self.results if r.status == TestStatus.FAIL)
        blocked = sum(1 for r in self.results if r.status == TestStatus.BLOCKED)
        skipped = sum(1 for r in self.results if r.status == TestStatus.SKIP)
        
        print("=" * 80)
        print(f"Summary:")
        print(f"  Total:   {total}")
        print(f"  Passed:  {passed}")
        print(f"  Failed:  {failed}")
        print(f"  Blocked: {blocked}")
        print(f"  Skipped: {skipped}")
        
        if total > 0:
            pass_rate = (passed / total) * 100
            print(f"  Pass rate: {pass_rate:.1f}%")
        
        print()
        
        if failed > 0:
            print("Failed tests:")
            for result in self.results:
                if result.status == TestStatus.FAIL:
                    print(f"  - {result.test.name}")
                    if result.error:
                        print(f"    {result.error[:100]}")
        
        return passed == total and failed == 0


def main():
    parser = argparse.ArgumentParser(
        description="A2oSX build validation suite for xasm++"
    )
    parser.add_argument(
        "--xasm",
        default="../../../build/src/xasm++",
        help="Path to xasm++ executable"
    )
    parser.add_argument(
        "--all",
        action="store_true",
        help="Run all tests including blocked ones"
    )
    parser.add_argument(
        "--known-good",
        action="store_true",
        help="Only run tests expected to pass"
    )
    parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Verbose output"
    )
    
    args = parser.parse_args()
    
    try:
        validator = A2oSXValidator(args.xasm)
        
        skip_blocked = not args.all
        if args.known_good:
            skip_blocked = True
        
        validator.run_all(skip_blocked=skip_blocked, verbose=args.verbose)
        
        # Exit with error if tests failed
        if any(r.status == TestStatus.FAIL for r in validator.results):
            sys.exit(1)
        
    except FileNotFoundError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"Unexpected error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
