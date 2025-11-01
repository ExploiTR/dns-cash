#!/usr/bin/env python3
"""
Fast DNS Performance Test Script
Tests DNS server response times and reliability
"""

import sys
import subprocess

# Auto-install dependencies
try:
    import dns.resolver
except ImportError:
    print("Installing dnspython...")
    subprocess.check_call([sys.executable, "-m", "pip", "install", "-q", "dnspython"])
    import dns.resolver

import time
import statistics


def dns_performance_test(host="127.0.0.1", port=6073, domain="google.com", queries=10):
    """
    Test DNS server performance
    
    Args:
        host: DNS server IP address
        port: DNS server port
        domain: Domain to query
        queries: Number of queries to perform
    """
    # Configure resolver
    resolver = dns.resolver.Resolver(configure=False)
    resolver.nameservers = [host]
    resolver.port = port
    resolver.timeout = 2
    resolver.lifetime = 2

    times = []
    failures = 0

    print(f"Testing DNS server at {host}:{port}")
    print(f"Query: {domain}")
    print(f"Total queries: {queries}\n")

    # Perform queries
    for i in range(1, queries + 1):
        start = time.perf_counter()
        try:
            answer = resolver.resolve(domain, "A")
            elapsed = (time.perf_counter() - start) * 1000
            times.append(elapsed)
            print(f"[{i}] SUCCESS {elapsed:.2f} ms | IP: {answer[0]}")
        except dns.exception.Timeout:
            failures += 1
            print(f"[{i}] FAILED (timeout)")
        except dns.resolver.NXDOMAIN:
            failures += 1
            print(f"[{i}] FAILED (domain not found)")
        except dns.resolver.NoAnswer:
            failures += 1
            print(f"[{i}] FAILED (no answer)")
        except Exception as e:
            failures += 1
            print(f"[{i}] FAILED ({type(e).__name__}: {str(e)})")

    # Print results
    print("\n" + "=" * 40)
    print("RESULTS")
    print("=" * 40)
    
    success = len(times)
    total = success + failures
    success_rate = (success / total * 100) if total > 0 else 0
    
    print(f"Success: {success}/{total} ({success_rate:.1f}%)")
    print(f"Failures: {failures}")
    
    if success > 0:
        print(f"\nTiming Statistics:")
        print(f"  Average: {statistics.mean(times):.2f} ms")
        print(f"  Minimum: {min(times):.2f} ms")
        print(f"  Maximum: {max(times):.2f} ms")
        if len(times) > 1:
            print(f"  Std Dev: {statistics.stdev(times):.2f} ms")
            print(f"  Median:  {statistics.median(times):.2f} ms")
    else:
        print("\nNo successful queries to calculate statistics.")


if __name__ == "__main__":
    # Default test configuration
    dns_performance_test(
        host="127.0.0.1",
        port=6073,
        domain="google.com",
        queries=10
    )
