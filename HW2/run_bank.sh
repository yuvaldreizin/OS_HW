#!/bin/bash

# Check if user provided number of ATMs
if [ -z "$1" ]; then
  echo "Usage: $0 <number_of_atms>"
  exit 1
fi

NUM_ATMS=$1

# Build the list of atm1 arguments
ARGS="atm1 atm0 atm2 atm3 "
for ((i=0; i<NUM_ATMS; i++)); do
  ARGS+="atm1 atm2 atm3 "
done

# Run the bank program with the generated arguments
NUM_ITERATIONS=$2
for ((i=0; i<NUM_ITERATIONS; i++)); do
    ./bank $ARGS 2> errors/err$i.txt
done