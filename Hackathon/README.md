# CPU and Memory Stress Testing Tool

A small command-line C program for Ubuntu/Linux that applies configurable CPU and memory load for a limited duration. It is intended for coursework and demonstrations, not as a certified hardware diagnostic tool.

## Features
- CPU workload using POSIX threads (`pthreads`)
- Configurable number of CPU worker threads
- Configurable memory allocation in MB
- Configurable run duration (1 to 3600 seconds)
- Input checks and a conservative memory-allocation guard
- Periodic progress output, operation count, and memory checksum
- Early stop with `Ctrl+C`

## Requirements
- Ubuntu/Linux (native Ubuntu or WSL2 Ubuntu)
- GCC and GNU Make
- POSIX threads support

## Install dependencies
```bash
sudo apt update
sudo apt install build-essential
```

## Compile
```bash
make
```

Alternative:
```bash
gcc -O2 -std=c11 -Wall -Wextra -Wpedantic main.c -o stress-test -pthread
```

## Run
```bash
./stress-test <seconds> <cpu_threads> <memory_MB>
```

Example:
```bash
./stress-test 30 4 512
```
This runs for up to 30 seconds, starts 4 CPU worker threads, and allocates 512 MB of memory. Choose a thread count no higher than the number of online CPU cores. The program rejects a memory request above half of the currently reported free RAM, but system memory availability can change while it runs.

A CPU-only test can be run with:
```bash
./stress-test 30 4 0
```

## Monitor the system
Open another terminal:
```bash
sudo apt install htop
htop
```
Or:
```bash
watch -n 1 free -h
```

## Clean generated binary
```bash
make clean
```

## Safety
- Start with a short duration and modest memory allocation.
- Save your work before running a test.
- Stop early with `Ctrl+C` if the system becomes unresponsive or unusually hot.
- Avoid allocating all available RAM. The memory guard is a basic check, not a guarantee against memory pressure.
- On WSL2, reported memory availability is subject to WSL configuration and host Windows memory pressure.
- This tool reports workload counters; it does not measure temperature, validate RAM integrity, or certify CPU stability.

## Project structure
```text
cpu-memory-stress-tool/
├── main.c
├── Makefile
├── README.md
├── PROJECT_REPORT.md
└── .gitignore
```

## Suggested test cases
| Test | Command | Expected result |
|---|---|---|
| Basic run | `./stress-test 5 1 64` | Runs and completes in about 5 seconds |
| Multiple threads | `./stress-test 10 2 128` | Reports CPU operations and completes |
| CPU-only | `./stress-test 5 1 0` | CPU workload runs without allocating memory |
| Invalid duration | `./stress-test 0 1 64` | Rejects invalid duration |
| Too many threads | `./stress-test 5 999999 64` | Rejects invalid thread count |
| Early stop | `./stress-test 60 2 128`, then `Ctrl+C` | Stops and prints summary |

Actual resource utilization and operation counts depend on the machine and workload; record observed values during your own test run rather than inventing benchmark results.
