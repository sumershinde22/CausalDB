#!/bin/bash

# CausalDB vs SQLite Benchmark Runner
# This script automates the entire benchmarking process

set -e  # Exit on any error

echo "🚀 CausalDB vs SQLite Benchmark Suite"
echo "====================================="

# Check if we're on macOS or Linux
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "📦 Detected macOS - checking for SQLite..."
    if ! brew list sqlite3 &>/dev/null; then
        echo "Installing SQLite via Homebrew..."
        brew install sqlite3
    fi
    # On macOS, SQLite headers are usually in /usr/include
    export CFLAGS="$CFLAGS -I/usr/include"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "📦 Detected Linux - checking for SQLite development libraries..."
    if ! dpkg -l | grep -q libsqlite3-dev; then
        echo "Installing SQLite development libraries..."
        sudo apt-get update
        sudo apt-get install -y libsqlite3-dev
    fi
else
    echo "⚠️  Unsupported OS: $OSTYPE"
    echo "Please install SQLite development libraries manually:"
    echo "  - macOS: brew install sqlite3"
    echo "  - Ubuntu/Debian: sudo apt-get install libsqlite3-dev"
    echo "  - CentOS/RHEL: sudo yum install sqlite-devel"
    exit 1
fi

echo "🔨 Building benchmark executable..."
make benchmark

if [ ! -f "./benchmark" ]; then
    echo "❌ Failed to build benchmark executable"
    exit 1
fi

echo "✅ Benchmark executable built successfully"

# Check if Python is available for analysis
if command -v python3 &> /dev/null; then
    echo "🐍 Python 3 found - will run detailed analysis"
    PYTHON_AVAILABLE=true
else
    echo "⚠️  Python 3 not found - will run basic benchmark only"
    PYTHON_AVAILABLE=false
fi

echo ""
echo "🏃 Running benchmark..."
echo "======================"

# Run the benchmark
./benchmark

echo ""
echo "📊 Benchmark completed!"

# Run Python analysis if available
if [ "$PYTHON_AVAILABLE" = true ]; then
    echo ""
    echo "📈 Running detailed analysis..."
    echo "=============================="
    
    # Check for matplotlib
    if python3 -c "import matplotlib" &>/dev/null; then
        echo "📊 matplotlib found - generating charts..."
        python3 analyze_performance.py
    else
        echo "⚠️  matplotlib not found - installing..."
        pip3 install matplotlib
        python3 analyze_performance.py
    fi
fi

echo ""
echo "🎉 Benchmark suite completed!"
echo ""
echo "📁 Generated files:"
echo "  - benchmark_causal.cdb (CausalDB test database)"
echo "  - benchmark_sqlite.db (SQLite test database)"
if [ "$PYTHON_AVAILABLE" = true ]; then
    echo "  - performance_report.md (Detailed analysis report)"
    echo "  - performance_comparison.png (Performance charts)"
fi
echo ""
echo "💡 To clean up test files, run: make clean" 