# CausalDB Project Structure

This document explains the organized directory structure of the CausalDB project.

## Directory Overview

### Core Source Code

- **`src/`** - Contains all C source files
  - `main.c` - Main CLI application entry point
  - `db.c` - Database operations and file I/O
  - `event.c` - Event data structure implementations
  - `statement.c` - SQL-like statement parsing
  - `repl.c` - Read-Eval-Print Loop for CLI

### Header Files

- **`include/`** - Contains all header files
  - `db.h` - Database interface declarations
  - `event.h` - Event structure definitions
  - `statement.h` - Statement parsing interface
  - `repl.h` - REPL interface declarations

### Build Artifacts

- **`build/`** - Contains compiled objects and executables
  - `*.o` - Compiled object files
  - `causaldb` - Main CLI executable
  - `benchmark` - Benchmark executable

### Web Application

- **`frontend/`** - Web frontend files
  - `index.html` - Main HTML page
  - `styles.css` - Modern CSS styling
  - `script.js` - Frontend JavaScript logic

### Server

- **`server/`** - HTTP server implementation
  - `server.c` - HTTP server code
  - `Makefile` - Server build configuration
  - `server` - Compiled server executable

### Benchmarking

- **`benchmarks/`** - Performance testing tools and results
  - `benchmark.c` - Performance benchmark code
  - `BENCHMARK_README.md` - Benchmark documentation
  - `BENCHMARK_RESULTS.md` - Benchmark results
  - `performance_report.md` - Performance analysis
  - `performance_comparison.png` - Performance charts

### Scripts and Utilities

- **`scripts/`** - Shell scripts and utilities
  - `run_benchmark.sh` - Automated benchmark runner
  - `analyze_performance.py` - Performance analysis script
  - `test_api.sh` - API testing script

### Data Storage

- **`data/`** - Database files
  - `causal.cdb` - Main database file
  - `benchmark_*.db` - Benchmark database files

### Documentation

- **`docs/`** - Additional documentation
  - `PROJECT_STRUCTURE.md` - This file

## Build Process

### Main Application

```bash
make                    # Build main CLI application
./build/causaldb        # Run CLI
```

### Server

```bash
cd server && make       # Build HTTP server
./server                # Run server
```

### Benchmark

```bash
make benchmark          # Build benchmark
./build/benchmark       # Run benchmark
```

## File Organization Benefits

1. **Separation of Concerns**: Source, headers, and build artifacts are clearly separated
2. **Clean Build**: Build artifacts are isolated in `build/` directory
3. **Easy Navigation**: Related files are grouped logically
4. **Scalability**: Easy to add new components without cluttering the root
5. **Maintenance**: Clear structure makes maintenance easier

## Development Workflow

1. **Edit Source**: Modify files in `src/` and `include/`
2. **Build**: Run `make` to compile
3. **Test**: Use executables in `build/`
4. **Benchmark**: Use tools in `benchmarks/` and `scripts/`
5. **Clean**: Run `make clean` to remove build artifacts
