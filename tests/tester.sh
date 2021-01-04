#!/bin/bash
set -eu

# Runs all tests
for N in $(seq 0 $1); do 
    ./run-test.sh  $N
done