#!/bin/bash

echo "Starting all test scenarios..."

# Running each test scenario script
for i in {1..10}; do
    echo "Running test scenario $i..."
    ./test_scenario_$i.sh
    if [ $? -eq 0 ]; then
        echo "Test scenario $i passed."
    else
        echo "Test scenario $i failed."
        echo "Test failed on test scenario $i. Exiting..."
        exit 1
    fi
done

echo "All test scenarios completed."
