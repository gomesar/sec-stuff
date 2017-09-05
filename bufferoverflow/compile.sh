#!/bin/bash
# remember to put '0' in randomize_va_space.
echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
# and to compile with -fno-stack-protector
gcc stackoverflow.c -o exec -fno-stack-protector
