#!/usr/bin/env bash

SERVER_NAME="localhost"

SERVER_PORT=49152  # iterative SV on 49152 port
#SERVER_PORT=49153  # concurrent SV on 49153 port

FILE_QTTY=256
FILE_SIZE=268435456

echo "Starting TCP clients to receive files..."

mkdir cl

t0=$(date +%s.%N)

pids=()

for i in $(seq 1 $FILE_QTTY); do
  ./tcp_cl $SERVER_NAME $SERVER_PORT "$i.bin" "./cl/$i.bin" $FILE_SIZE&
  pids+=($!)
done

for pid in "${pids[@]}"; do
    wait "$pid"
    if [ $? -ne 0 ]; then
        echo "tcp_cl ($pid) failed!"
    fi
done

t1=$(date +%s.%N)

total=$(echo "$t1 - $t0" | bc)

echo "Sending/receiving time: $total seconds."

#echo "Removing files from clients..."
#
#rm cl/*.bin
#rmdir cl
