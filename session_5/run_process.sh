#!/bin/bash

echo "Starting 20 processes..."

for i in $(seq 0 19)
do
    /home/hungubuntu/Vim_C_code/Embedded_Training/session_5/process "$i" &
done

echo
echo "All processes started."
echo

wait
