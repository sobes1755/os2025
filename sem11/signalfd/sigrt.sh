#!/usr/bin/env bash

SIGRTMIN=34
SIGRTINT=77
SIGKILL=9

./signalfd $SIGRTMIN &
PID=$!
sleep 1
./killqueue $PID $SIGRTMIN $SIGRTINT
sleep 1
./killqueue $PID $SIGKILL
sleep 1
