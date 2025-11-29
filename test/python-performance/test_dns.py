#!/usr/bin/env python3
"""
High-Performance DNS Load Test with Real-Time Progress
"""

import sys
import subprocess

# Auto-install dependencies
try:
    import dns.asyncresolver
    from tqdm.asyncio import tqdm_asyncio
except ImportError:
    print("Installing dependencies...")
    subprocess.check_call([sys.executable, "-m", "pip", "install", "-q", "dnspython", "tqdm"])
    import dns.asyncresolver
    from tqdm.asyncio import tqdm_asyncio

import asyncio
import time
from collections import Counter


async def query_dns(resolver, domain, stats):
    """Single DNS query"""
    try:
        await resolver.resolve(domain, "A")
        stats["success"] += 1
    except dns.asyncresolver.NoAnswer:
        stats["no_answer"] += 1
    except dns.asyncresolver.NXDOMAIN:
        stats["nxdomain"] += 1
    except dns.exception.Timeout:
        stats["timeout"] += 1
    except Exception:
        stats["error"] += 1


async def run_load_test(
    host="127.0.0.1",
    port=6073,
    domain="google.com",
    total_queries=10000,
    concurrency=1000,
):
    """DNS load test with progress bar"""
    print(f"DNS Load Test")
    print(f"=" * 50)
    print(f"Target:      {host}:{port}")
    print(f"Domain:      {domain}")
    print(f"Queries:     {total_queries:,}")
    print(f"Concurrency: {concurrency}")
    print(f"=" * 50)
    print()
    
    # Configure resolver
    resolver = dns.asyncresolver.Resolver(configure=False)
    resolver.nameservers = [host]
    resolver.port = port
    resolver.timeout = 5.0
    resolver.lifetime = 5.0
    
    # Stats tracking
    stats = Counter()
    
    # Semaphore for concurrency control
    semaphore = asyncio.Semaphore(concurrency)
    
    async def throttled_query():
        async with semaphore:
            await query_dns(resolver, domain, stats)
    
    # Create tasks
    tasks = [throttled_query() for _ in range(total_queries)]
    
    # Execute with progress bar
    start_time = time.perf_counter()
    await tqdm_asyncio.gather(*tasks, desc="Queries", unit="req")
    elapsed = time.perf_counter() - start_time
    
    # Results
    total = sum(stats.values())
    qps = total / elapsed if elapsed > 0 else 0
    
    print(f"\n{'=' * 50}")
    print(f"RESULTS")
    print(f"{'=' * 50}")
    print(f"Duration:    {elapsed:.2f}s")
    print(f"Total:       {total:,} queries")
    print(f"Success:     {stats['success']:,} ({stats['success']/total*100:.1f}%)")
    print(f"Timeouts:    {stats['timeout']:,}")
    print(f"NXDomain:    {stats['nxdomain']:,}")
    print(f"No Answer:   {stats['no_answer']:,}")
    print(f"Errors:      {stats['error']:,}")
    print(f"\n{'=' * 50}")
    print(f"THROUGHPUT:  {qps:,.0f} queries/second")
    print(f"{'=' * 50}")


if __name__ == "__main__":
    asyncio.run(run_load_test(
        host="127.0.0.1",
        port=6073,
        domain="google.com",
        total_queries=100000,
        concurrency=5000,
    ))
