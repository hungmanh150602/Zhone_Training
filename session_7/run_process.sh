#!/bin/bash

echo "Starting 20 processes..."

for i in $(seq 0 100)
do
    ./client "10.0.0.81" "1234" &
done

echo
echo "All processes started."
echo

wait
