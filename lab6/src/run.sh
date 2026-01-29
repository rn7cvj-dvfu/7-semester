#!/bin/bash

echo "================================"
echo "Temperature Monitor GUI"
echo "================================"

cd "$(dirname "$0")/build"

if [ ! -f "temperature_monitor" ]; then
    echo "Error: Executable not found!"
    echo "Please run ./build.sh first"
    exit 1
fi

echo ""
echo "Starting Temperature Monitor..."
echo ""
echo "Make sure the server from lab5 is running on http://localhost:8080"
echo ""

./temperature_monitor
