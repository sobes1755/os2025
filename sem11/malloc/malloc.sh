#!/usr/bin/env bash

./malloc &
PID=$!
sleep 1

for sig1 in {1..1024}
do
    kill -s SIGUSR1 $PID
    if [ $? -ne 0 ]; then break; fi
done
sleep 1

for sig2 in {1..1024}
do
    kill -s SIGUSR2 $PID
    if [ $? -ne 0 ]; then break; fi
done
sleep 1

kill -s SIGTERM $PID
sleep 0.5
kill -s SIGTERM $PID
sleep 0.5
kill -s SIGTERM $PID
sleep 0.5
