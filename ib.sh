#!/bin/bash

usage() {
	  echo "Usage: ib.sh (start|stop)"
}

if [[ $EUID -ne 0 ]]; then
    echo "This script must be run as root" 1>&2
    exit 1
fi

if [[ $# -ne 1 ]]; then
    usage
	  exit 1
fi

if [[ "$1" == "start" ]]; then
	  service rdma start
	  service opensmd start

    ip addr add 10.0.100.100/24 dev ib0
    ip link set ib0 up

elif [[ "$1" == "stop" ]]; then
	  service opensmd stop
	  service rdma stop
else
    usage
	  exit 1
fi
