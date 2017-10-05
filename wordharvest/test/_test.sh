#!/bin/bash
../bin//wordharvest -d "./" -o wordlist -e "text:txt:asc"
../bin/bruteforce -l wordlist -f file.zip
