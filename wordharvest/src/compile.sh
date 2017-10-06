#!/bin/bash
if [ ! -d ../bin/ ];then
	echo "Creating '../bin' folder"
	mkdir -p ../bin
fi
gcc wordharvest.c -o ../bin/wordharvest
gcc bruteforce.c -o ../bin/bruteforce

# Comment the lines below to not generate dummy files and tip
unzip -o dummy.zip -d ../
echo "[!] Go to ../test/ and run ./_test.sh"
#cd ../test/; ./_test.sh; cd ../src;

