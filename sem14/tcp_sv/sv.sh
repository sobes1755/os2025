#!/usr/bin/env bash

SERVER_NAME="localhost"
SERVER_PORT=49152
SERVER_DIRNAME="./sv"

FILE_QTTY=256
FILE_SIZE="64M"

#echo "Creating 256 64MB files on server..."
#
#mkdir sv
#
#for i in $(seq 1 $FILE_QTTY); do
#    head -c $FILE_SIZE /dev/random > ${SERVER_DIRNAME}/$i.bin
#done

echo "Starting TCP server..."

./tcp_sv $SERVER_NAME $SERVER_PORT $SERVER_DIRNAME

#echo "Removing files from server..."
#
#rm sv/*.bin
#rmdir sv
