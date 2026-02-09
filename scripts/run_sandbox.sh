#!/bin/bash
# Helper script to build and run PyEngine sandbox

set -e

cd "$(dirname "$0")/.."

echo "=== Building PyEngine ==="
cmake --build build

echo ""
echo "=== Running Sandbox ==="
./build/bin/PyEngineSandbox
