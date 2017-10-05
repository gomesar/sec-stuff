#!/bin/bash
if [ ! -d ../bin/ ];then
	echo "Creating '../bin' folder"
	mkdir -p ../bin
fi
gcc wordharvest.c -o ../bin/wordharvest
gcc bruteforce.c -o ../bin/bruteforce

#../test/_test.sh

