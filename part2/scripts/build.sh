#!/bin/bash

DIRECTORY="/home/jaison/CS7610/project1/part2/src"
CLEAN_AND_BUILD="make clean && make build && make generate"

ssh "jaison@login.ccs.neu.edu" "cd $DIRECTORY" $CLEAN_AND_BUILD;
