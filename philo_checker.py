#!/usr/bin/env python3

import os
import sys
import time
import signal
import subprocess
from typing import List, Dict, Tuple, Optional


class TestCase:
    def __init__(
        self, name: str, args: List[str], expected_result: str, timeout: int = 5
    ):
        self.name = name
        self.args = args
        self.expected_result = expected_result  # "die" or "complete"
        self.timeout = timeout
        self.output = ""
        self.result = None
        self.execution_time = 0

    def __str__(self):
        return f"Test: {self.name}\nArgs: {' '.join(self.args)}\nExpected: {self.expected_result}"


class PhilosophersTestSuite:
    def __init__(self, binary_path: str):
        self.binary_path = binary_path
        self.tests = []
        self.results = {"passed": 0, "failed": 0, "timeout": 0}

    def add_test(self, test: TestCase):
        self.tests.append(test)

    def generate_standard_tests(self):
        """Generate standard test cases for typical scenarios"""
        # Basic death test
        self.add_test(
            TestCase(
                name="One philosopher must die",
                args=["1", "800", "200", "200"],
                expected_result="die",
                timeout=2,
            )
        )

        # No death test (enough time)
        self.add_test(
            TestCase(
                name="No death with enough time",
                args=["4", "410", "200", "200"],
                expected_result="running",
                timeout=3,
            )
        )

        # Death test (not enough time)
        self.add_test(
            TestCase(
                name="Death with tight timing",
                args=["4", "310", "200", "200"],
                expected_result="die",
                timeout=2,
            )
        )

        # Meals limit test
        self.add_test(
            TestCase(
                name="All philosophers eat N times",
                args=["5", "800", "200", "200", "3"],
                expected_result="complete",
                timeout=5,
            )
        )

        # Edge cases
        self.add_test(
            TestCase(
                name="Minimum values test",
                args=["2", "60", "30", "30"],
                expected_result="running",
                timeout=2,
            )
        )

        # Performance test
        self.add_test(
            TestCase(
                name="Many philosophers performance",
                args=["20", "800", "200", "200"],
                expected_result="running",
                timeout=5,
            )
        )

        # Invalid inputs (should be handled gracefully)
        self.add_test(
            TestCase(
                name="Negative time value",
                args=["4", "-200", "200", "200"],
                expected_result="error",
                timeout=1,
            )
        )

    def run_test(self, test: TestCase) -> bool:
        """Run a single test case and return True if passed"""
        print(f"\n--- Running test: {test.name} ---")
        print(f"Command: {self.binary_path} {' '.join(test.args)}")

        cmd = [self.binary_path] + test.args
        start_time = time.time()

        try:
            if test.expected_result == "running":
                # For tests expected to keep running without death
                proc = subprocess.Popen(
                    cmd,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    text=True,
                    preexec_fn=os.setsid,
                )

                # Let it run for a bit
                time.sleep(test.timeout - 0.5)

                # Check if still running
                if proc.poll() is None:
                    # Still running - good!
                    os.killpg(os.getpgid(proc.pid), signal.SIGTERM)
                    test.result = "passed"
                    test.output = "Process was still running as expected"
                else:
                    stdout, stderr = proc.communicate()
                    test.output = stdout + stderr
                    test.result = "failed"
                    print(
                        f"Process terminated unexpectedly. Exit code: {proc.returncode}"
                    )
                    print(f"Output: {test.output[:200]}...")
            else:
                # For tests expecting completion or death
                result = subprocess.run(
                    cmd, capture_output=True, text=True, timeout=test.timeout
                )
                test.output = result.stdout + result.stderr

                if test.expected_result == "die" and "died" in test.output:
                    test.result = "passed"
                elif test.expected_result == "complete" and "died" not in test.output:
                    test.result = "passed"
                elif test.expected_result == "error" and result.returncode != 0:
                    test.result = "passed"
                else:
                    test.result = "failed"
                    print(f"Test failed. Output: {test.output[:200]}...")

        except subprocess.TimeoutExpired:
            if test.expected_result == "running":
                test.result = "passed"
                test.output = "Process was still running as expected (timeout)"
            else:
                test.result = "timeout"
                print(f"Test timed out after {test.timeout} seconds")

        except Exception as e:
            test.result = "failed"
            test.output = f"Exception: {str(e)}"
            print(f"Test error: {e}")

        test.execution_time = time.time() - start_time

        if test.result == "passed":
            self.results["passed"] += 1
            print(f"✅ Test PASSED in {test.execution_time:.2f}s")
            return True
        elif test.result == "timeout":
            self.results["timeout"] += 1
            print(f"⏱️ Test TIMEOUT in {test.execution_time:.2f}s")
            return False
        else:
            self.results["failed"] += 1
            print(f"❌ Test FAILED in {test.execution_time:.2f}s")
            return False

    def run_all_tests(self) -> Dict:
        """Run all test cases and return results summary"""
        if not os.path.exists(self.binary_path):
            print(f"Error: Binary {self.binary_path} does not exist")
            sys.exit(1)

        print(f"Running {len(self.tests)} tests on {self.binary_path}")

        for test in self.tests:
            self.run_test(test)

        print("\n--- Test Suite Results ---")
        print(f"Total tests: {len(self.tests)}")
        print(f"Passed: {self.results['passed']}")
        print(f"Failed: {self.results['failed']}")
        print(f"Timeout: {self.results['timeout']}")

        return self.results

    def analyze_output_file(self, output_file: str) -> Tuple[bool, List[str]]:
        """Analyze a log file containing philosopher output"""
        try:
            with open(output_file, "r") as f:
                content = f.read()

            # Use the PhilosophersChecker to analyze the output
            # This requires importing the checker from the other file
            # Or you could implement a simpler version here

            return True, ["Output analysis not implemented"]
        except Exception as e:
            return False, [f"Error analyzing output: {str(e)}"]


def main():
    if len(sys.argv) < 2:
        print("Usage: test_suite.py path_to_philo_binary")
        sys.exit(1)

    binary_path = sys.argv[1]
    test_suite = PhilosophersTestSuite(binary_path)

    # Generate standard test cases
    test_suite.generate_standard_tests()

    # Add custom test cases if needed
    # test_suite.add_test(TestCase(...))

    # Run all tests
    results = test_suite.run_all_tests()

    # Exit with appropriate status code
    sys.exit(0 if results["failed"] == 0 and results["timeout"] == 0 else 1)


if __name__ == "__main__":
    main()
