#!/bin/bash

echo "Starting 20 processes..."

for i in $(seq 0 19)
do
    ./process "$i" &
done

echo
echo "All processes started."
echo

wait
