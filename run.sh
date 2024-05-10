#!/bin/bash	

# Compile your program (if necessary)
gcc cruz_lab04.c

# Define the number of runs
total_runs=2

# Define the input value for the remaining runs
# master_inputs="5 8080 0"
slave_inputs="10 8080 1"

# ./a.out $master_inputs

# Run the program with the same input value for the remaining runs
for ((i=1; i<=total_runs; i++)); do
    ./a.out $slave_inputs
done

