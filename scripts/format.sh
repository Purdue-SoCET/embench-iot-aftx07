#!/bin/sh

echo "$1.cycles = ["
grep "^Clocks: " $1 | awk '{printf "    %s\n", $2}'
echo "];"

echo "$1.instrs = ["
grep "^Instructions: " $1 | awk '{printf "    %s\n", $2}'
echo "];"

echo "$1.speed = ["
tail -n 24 $1 | head -n 19 | awk '{printf "    %s\n", $2}'
echo "];"

echo "Benchmark names for $1"
echo "["
tail -n 24 $1 | head -n 19 | awk '{printf "    \"%s\"\n", $1}'
echo "]"
