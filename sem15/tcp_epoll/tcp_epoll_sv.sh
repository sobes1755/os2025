#!/usr/bin/env bash

SERVER_IP=$(hostname -I | awk '{print $1}')
SERVER_PORT=49152

./tcp_epoll_sv $SERVER_IP $SERVER_PORT
