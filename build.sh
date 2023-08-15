#!/bin/bash

# Check if the source file name is provided
if [ "$#" -ne 2 ]; then
    echo "Usage: ./build.sh <source_file_name.c> <output_file_name>"
    exit 1
fi

# Source file name
SOURCE_FILE=$1

# Output file name
OUTPUT_FILE=$2

# SQLite paths for Homebrew on Apple Silicon Macs
SQLITE_INCLUDE_PATH="/opt/homebrew/opt/sqlite/include"
SQLITE_LIB_PATH="/opt/homebrew/opt/sqlite/lib"

# Compile the program
gcc $SOURCE_FILE -I$SQLITE_INCLUDE_PATH -L$SQLITE_LIB_PATH -lsqlite3 -o $OUTPUT_FILE

# Check if the compilation was successful
if [ "$?" -eq 0 ]; then
    echo "Compilation successful! Executable name: $OUTPUT_FILE"
else
    echo "Compilation failed."
    exit 2
fi