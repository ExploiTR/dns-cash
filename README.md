## Overview

`dns-cash` is designed to be a high-performance, multi-threaded DNS server. It features a modular architecture separating the server listener, request processor, and execution engine.

This project is currently under active development. While the core infrastructure (server, threading, caching) is in place, the logic for handling DNS responses and forwarding is incomplete.

A personal experimental project implementing a DNS server in C++ adhering to [RFC 1035](https://datatracker.ietf.org/doc/html/rfc1035).

## Project Status

### Implemented Features
- [x] **High-Performance UDP Server**: Uses Windows Sockets (WSA) for handling UDP traffic.
- [x] **Thread Pool**: Custom thread pool implementation for concurrent request processing.
- [x] **TLRU Cache**: Time-Aware Least Recently Used cache to store DNS records with TTL support.
- [x] **DNS Parsing**:
    - [x] Header Section Parsing
    - [x] Question Section Parsing
    - [x] Message Compression (RFC 1035 4.1.4) support

### To-Do / In Progress
- [ ] **Response Generation**: Logic to construct and serialize DNS response packets.
- [ ] **Upstream Forwarding**:
    - [ ] Handling responses from upstream resolvers (e.g., 8.8.8.8).
    - [ ] Mapping upstream responses back to original client requests (Transaction ID matching).
- [ ] **Full Packet Parsing**: Parsing Answer, Authority, and Additional sections.
- [ ] **Client Reply**: Sending the resolved (or cached) answer back to the client.
- [ ] **TCP Support**: Fallback to TCP for large queries/responses (RFC 1035 requirement).

## Project Structure

- `src/server`: UDP server implementation (`DNSServer`).
- `src/processor`: DNS packet parsing and logic (`dns_parser`, `dns_processor`).
- `src/executors`: Concurrency management (`ThreadPool`).
- `src/utils`: Utility structures like `TLRUCache` and command-line parsing.
- `src/callback`: Request handling callbacks.

## Build Instructions

### Prerequisites
- CMake 3.10 or higher
- C++20 compatible compiler (e.g., MSVC, GCC, Clang)

### Building
```bash
mkdir out
cd out
cmake ..
cmake --build .
```

## Usage

Run the generated executable:
```bash
./dns-cash
```
By default, the server listens on port **6073**.