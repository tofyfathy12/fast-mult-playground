#!/usr/bin/env python3
"""
test_algorithms.py — Correctness tester for fast multiplication algorithms.

Runs each compiled executable against a suite of test cases (multiplication,
power, factorial) and compares the output against Python's native arithmetic.
"""

import subprocess
import sys
import os
import random
import math
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent.parent

EXECUTABLES = {
    # name: (path, supports_factorial, supports_power)
    "ntt_v1":       (PROJECT_ROOT / "ntt" / "ntt_v1.exe",       True,  True),
    "ntt_v2":       (PROJECT_ROOT / "ntt" / "ntt_v2.exe",       True,  True),
    "ntt_v3":       (PROJECT_ROOT / "ntt" / "ntt_v3.exe",       True,  True),
    "ntt_v4":       (PROJECT_ROOT / "ntt" / "ntt_v4.exe",       True,  True),
    "ntt_v5":       (PROJECT_ROOT / "ntt" / "ntt_v5.exe",       True,  True),
    "fft_v1":       (PROJECT_ROOT / "fft" / "fft_v1.exe",       False, True),
    "fft_v2":       (PROJECT_ROOT / "fft" / "fft_v2.exe",       False, True),
    "karatsuba_v0": (PROJECT_ROOT / "karatsuba" / "karatsuba_v0.exe", True,  False),
    "karatsuba_v1": (PROJECT_ROOT / "karatsuba" / "karatsuba_v1.exe", False, True),
}

def generate_random_number(digits):
    """Generate a random number string with exactly `digits` digits."""
    if digits == 1:
        return str(random.randint(1, 9))
    first = str(random.randint(1, 9))
    rest = ''.join(str(random.randint(0, 9)) for _ in range(digits - 1))
    return first + rest

MULT_CASES = [
    # (num1, num2, description)
    ("2", "3", "tiny"),
    ("0", "999", "zero"),
    ("123", "456", "small"),
    ("999", "999", "carry chain"),
    ("12345", "67890", "5-digit"),
    ("9999999999", "9999999999", "10-digit worst case"),
    ("123456789012345", "987654321098765", "15-digit"),
    (generate_random_number(50), generate_random_number(50), "50-digit random"),
    (generate_random_number(100), generate_random_number(100), "100-digit random"),
    (generate_random_number(200), generate_random_number(200), "200-digit random"),
]

POWER_CASES = [
    # (base, exponent, description)
    ("2", 10, "2^10"),
    ("3", 20, "3^20"),
    ("99", 5, "99^5"),
    ("12345", 3, "12345^3"),
    ("2", 50, "2^50"),
]

FACTORIAL_CASES = [
    # (n, description)
    (5, "5!"),
    (10, "10!"),
    (20, "20!"),
    (50, "50!"),
    (100, "100!"),
]

def run_exe(exe_path, args, timeout=30):
    """Run an executable with args and return stdout stripped."""
    try:
        result = subprocess.run(
            [str(exe_path)] + args,
            capture_output=True, text=True, timeout=timeout
        )
        return result.stdout.strip(), result.returncode
    except subprocess.TimeoutExpired:
        return None, -1
    except FileNotFoundError:
        return None, -2

def parse_mult_result(output):
    """Extract the result from '<num1> * <num2> = <result>'."""
    for line in output.splitlines():
        if '*' in line and '=' in line:
            return line.split('=')[-1].strip()
    return None

def parse_pow_result(output):
    """Extract the result from '<base> ^ <exp> = <result>'."""
    for line in output.splitlines():
        if '^' in line and '=' in line:
            return line.split('=')[-1].strip()
    return None

def parse_fact_result(output):
    """Extract the result from '<n>! = <result>'."""
    for line in output.splitlines():
        if '!' in line and '=' in line:
            return line.split('=')[-1].strip()
    return None

class TestStats:
    def __init__(self):
        self.passed = 0
        self.failed = 0
        self.skipped = 0
        self.errors = []

    def record(self, passed, name="", detail=""):
        if passed:
            self.passed += 1
        else:
            self.failed += 1
            self.errors.append(f"  FAIL: {name} — {detail}")

    def skip(self):
        self.skipped += 1

    def summary(self):
        total = self.passed + self.failed
        print(f"\n{'='*60}")
        print(f"  Results: {self.passed}/{total} passed, {self.failed} failed, {self.skipped} skipped")
        if self.errors:
            print(f"\n  Failures:")
            for e in self.errors:
                print(e)
        print(f"{'='*60}")
        return self.failed == 0


def test_multiplication(stats):
    print("\n" + "─" * 60)
    print("  MULTIPLICATION TESTS")
    print("─" * 60)
    for num1, num2, desc in MULT_CASES:
        expected = str(int(num1) * int(num2))
        for name, (exe, _, _) in EXECUTABLES.items():
            if not exe.exists():
                stats.skip()
                continue
            output, code = run_exe(exe, [num1, num2])
            if code != 0 or output is None:
                label = f"{name} mult ({desc})"
                stats.record(False, label, f"exit code {code}")
                print(f"  ✗ {label}: crashed (exit {code})")
                continue
            actual = parse_mult_result(output)
            label = f"{name} mult ({desc})"
            if actual == expected:
                stats.record(True)
                print(f"  ✓ {label}")
            else:
                stats.record(False, label, f"expected {expected[:40]}..., got {str(actual)[:40]}...")
                print(f"  ✗ {label}: expected {expected[:40]}..., got {str(actual)[:40]}...")


def test_power(stats):
    print("\n" + "─" * 60)
    print("  POWER TESTS")
    print("─" * 60)
    for base, exp, desc in POWER_CASES:
        expected = str(int(base) ** exp)
        for name, (exe, _, supports_pow) in EXECUTABLES.items():
            if not supports_pow or not exe.exists():
                stats.skip()
                continue
            output, code = run_exe(exe, ["-p", base, str(exp)])
            if code != 0 or output is None:
                label = f"{name} pow ({desc})"
                stats.record(False, label, f"exit code {code}")
                print(f"  ✗ {label}: crashed (exit {code})")
                continue
            actual = parse_pow_result(output)
            label = f"{name} pow ({desc})"
            if actual == expected:
                stats.record(True)
                print(f"  ✓ {label}")
            else:
                stats.record(False, label, f"expected {expected[:40]}..., got {str(actual)[:40]}...")
                print(f"  ✗ {label}: expected {expected[:40]}..., got {str(actual)[:40]}...")


def test_factorial(stats):
    print("\n" + "─" * 60)
    print("  FACTORIAL TESTS")
    print("─" * 60)
    for n, desc in FACTORIAL_CASES:
        expected = str(math.factorial(n))
        for name, (exe, supports_fact, _) in EXECUTABLES.items():
            if not supports_fact or not exe.exists():
                stats.skip()
                continue
            output, code = run_exe(exe, ["-f", str(n)])
            if code != 0 or output is None:
                label = f"{name} fact ({desc})"
                stats.record(False, label, f"exit code {code}")
                print(f"  ✗ {label}: crashed (exit {code})")
                continue
            actual = parse_fact_result(output)
            label = f"{name} fact ({desc})"
            if actual == expected:
                stats.record(True)
                print(f"  ✓ {label}")
            else:
                stats.record(False, label, f"expected {expected[:40]}..., got {str(actual)[:40]}...")
                print(f"  ✗ {label}: expected {expected[:40]}..., got {str(actual)[:40]}...")


def main():
    print("╔══════════════════════════════════════════════════════════╗")
    print("║     Fast Multiplication Algorithms — Correctness Test    ║")
    print("╚══════════════════════════════════════════════════════════╝")

    stats = TestStats()
    test_multiplication(stats)
    test_power(stats)
    test_factorial(stats)

    all_passed = stats.summary()
    sys.exit(0 if all_passed else 1)


if __name__ == "__main__":
    main()
