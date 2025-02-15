#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <executable> <N>"
    exit 1
fi

EXECUTABLE=$1
N=$2

# Check if the executable exists and is executable
if [ ! -x "$EXECUTABLE" ]; then
    echo "Error: $EXECUTABLE is not an executable file."
    exit 1
fi

# Start timing the overall execution
START_TIME=$(date +%s.%N)

for ((i=0; i<N; i++)); do
    $EXECUTABLE > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "Error: Execution of $EXECUTABLE failed."
        exit 1
    fi
done

# End timing
END_TIME=$(date +%s.%N)

# Calculate elapsed time
TOTAL_TIME=$(echo "$END_TIME - $START_TIME" | bc)
AVERAGE_TIME=$(echo "$TOTAL_TIME / $N" | bc -l)

# Display results
echo "Total elapsed time: $TOTAL_TIME seconds"
echo "Average time per execution: $AVERAGE_TIME seconds"
