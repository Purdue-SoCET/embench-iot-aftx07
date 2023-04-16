#!/bin/sh

echo "Instructions for $1"
echo "["
grep "^Instructions: " $1 | awk '{printf "    %s\n", $2}'
echo "]"

echo "Clocks for $1"
echo "["
grep "^Clocks: " $1 | awk '{printf "    %s\n", $2}'
echo "]"

echo "Speed for $1"
echo "["
tail -n 24 $1 | head -n 19 | awk '{printf "    %s\n", $2}'
echo "]"

echo "Benchmark names for $1"
echo "["
tail -n 24 $1 | head -n 19 | awk '{printf "    \"%s\"\n", $1}'
echo "]"
