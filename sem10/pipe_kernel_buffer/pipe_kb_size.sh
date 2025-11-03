#!/usr/bin/env bash

./pipe_kb_size &

PID=$!

sleep 5

echo
echo
echo "Process $PID has opened the following descriptors (see /proc/PID/fd)."
echo 
ls -l /proc/$PID/fd | sed "1d"

echo
echo -n "Process $PID is waiting on "
cat /proc/$PID/wchan
echo " (see /proc/PID/wchan)..."

echo
echo "Sending SIGSTOP and SIGCONT to process $PID to return from waiting..."
echo

kill -s SIGSTOP $PID
kill -s SIGCONT $PID
