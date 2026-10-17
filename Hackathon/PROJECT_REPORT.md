# Project Report: CPU and Memory Stress Testing Tool

## 1. Abstract
The CPU and Memory Stress Testing Tool is a Linux command-line application written in C. It generates configurable CPU workload using POSIX threads and repeatedly accesses a user-selected memory region. The user controls the duration, number of CPU worker threads, and memory allocation. The program reports elapsed time, a CPU operation counter, and a memory checksum.

## 2. Problem Statement
Users and students need a simple way to apply controlled CPU and memory load on Ubuntu to observe system behaviour under resource pressure. Existing command-line utilities can be powerful but may be unfamiliar to beginners. This project provides a small, transparent educational prototype with command-line configuration and basic safety checks.

## 3. Objectives
- Create a working stress-test program for Ubuntu/Linux.
- Allow duration, CPU thread count, and memory size to be configured.
- Use C, GCC, pthreads, and Make.
- Validate user input and reject excessive memory requests relative to reported free RAM.
- Display progress and a completion summary.
- Document installation, compilation, execution, and testing.

## 4. Technologies
- C (C11): core implementation
- GCC: compiler
- POSIX threads (`pthreads`): parallel CPU workers
- C11 atomics: shared stop flag and operation counter
- GNU Make: build automation
- Ubuntu/Linux: target environment
- `htop` and `free`: external monitoring during demonstrations

## 5. System Design
### Inputs
- Duration in seconds (1-3600)
- CPU worker thread count (1 through the number of online CPUs)
- Memory allocation in MB (0 or more, limited to half of currently reported free RAM)

### Processing
1. Parse and validate command-line arguments.
2. Read system information and apply the memory guard.
3. Allocate and initialize the requested memory region.
4. Create CPU worker threads.
5. Run memory read/write activity until the time limit or an interrupt.
6. Stop and join worker threads.
7. Print elapsed time, operation count, and checksum; free allocated resources.

### Outputs
- Progress line while running
- Completion or early-stop status
- Elapsed seconds
- CPU operation count
- Memory checksum

## 6. Implementation Approach
Each CPU worker repeatedly performs integer arithmetic and increments a shared atomic counter. The main thread touches pages of the allocated memory region and reads samples to calculate a checksum. The stop flag allows workers to exit cleanly. `SIGINT` and `SIGTERM` request an early stop. The Makefile compiles the program with warnings enabled.

## 7. Build and Execution
```bash
sudo apt update
sudo apt install build-essential
make
./stress-test 30 4 512
```
The example requests 30 seconds, four CPU threads, and 512 MB of memory. Select values appropriate for the target machine.

## 8. Testing Plan and Results
Run the following cases and record the actual result on the target system:
- Valid short test: `./stress-test 5 1 64`
- Multiple CPU threads: `./stress-test 10 2 128`
- CPU-only: `./stress-test 5 1 0`
- Invalid duration: `./stress-test 0 1 64`
- Invalid thread count: `./stress-test 5 999999 64`
- Early stop: start a longer run and press `Ctrl+C`

Expected outcomes are that valid inputs run and print a summary, invalid settings are rejected, and an interrupt requests a clean stop. Do not claim measured performance until you have run these tests and captured the output. Use `htop` and `free -h` in another terminal to observe system load.

## 9. Limitations
- Operation counts are not a standardized performance score and are not directly comparable across different machines.
- The memory checksum is not a complete RAM integrity test.
- No CPU temperature, frequency, throttling, or error-correction data is collected.
- Free memory can change after validation; the guard cannot guarantee that the operating system will never experience memory pressure.
- WSL2 resource reporting and behaviour depend on the host and WSL configuration.

## 10. Future Enhancements
- Add CSV logging with timestamps and system metadata.
- Add temperature monitoring where supported.
- Add selectable workload profiles and repeatable benchmark modes.
- Add memory verification patterns and a structured test summary.
- Add a graphical dashboard.

## 11. Conclusion
The prototype demonstrates CPU concurrency, memory allocation and access, command-line validation, signal handling, and build automation on Ubuntu. It is suitable as an educational project and a foundation for more rigorous system testing.
