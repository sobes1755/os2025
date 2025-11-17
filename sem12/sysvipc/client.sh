#!/usr/bin/env bash

for mul1 in {1..1024}
do
for mul2 in {1..1024}
do
    ./client $mul1 $mul2
done
done
