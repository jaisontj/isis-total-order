#!/bin/bash

DIRECTORY="/home/jaison/CS7610/project1/part2/src"
COMMAND_RUN="make --directory=$DIRECTORY run"
COMMAND_RUN_SNAP="make --directory=$DIRECTORY runsnap"

for host in $(cat hostnames.txt); do
	if [ "$host" = "vdi-linux-030.ccs.neu.edu" ]; then
		ssh $host $COMMAND_RUN_SNAP &
	else
		ssh $host $COMMAND_RUN &
	fi
done
wait

