#!/usr/bin/env bash

IANALOOPBACK6="::1"
IANAEPHEMERAL=49152

./udpecho_sv $IANALOOPBACK6 $IANAEPHEMERAL&
PID=$!

sleep 1
netstat -ulnp 2>/dev/null
sleep 1
./udpecho_cl $IANALOOPBACK6 $IANAEPHEMERAL "Reliable TCP,"
sleep 1
./udpecho_cl $IANALOOPBACK6 $IANAEPHEMERAL "Subtle, cute UDP,"
sleep 1
./udpecho_cl $IANALOOPBACK6 $IANAEPHEMERAL "Internet needs both."
sleep 1

kill $PID
