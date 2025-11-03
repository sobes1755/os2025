#!/usr/bin/env bash

echo "Col 1: qtty of processes writing to pipe"
echo "Col 2: size of write buffer in PIPE_BUFs"
echo "Col 3: qtty of detected nonatomic writes"

for b in {1..3}; do
for p in {1..3}; do
    echo -n "$p "
    echo -n "$b "
    ./pipe_ab_size $p $b | tee "$p-$b.txt" | grep -E -v '^(.)\1*$' | wc -l;
done;
done;
