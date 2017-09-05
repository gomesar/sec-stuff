## How to:
```
# ./compile.sh
```
or (after turn off Address Space Layout Randomization (ASLR) putting '0' in '/proc/sys/kernel/randomize_va_space') *:
```
$ gcc stackoverflow.c -o exec -fno-stack-protector
```

play with it:
```
$ ./exec whereami
```

-----
Exploitation example refered on pdf:
```
$ perl trick.pl
```

\*REMEBER: [It is dangerous to keep ASLR OFF, turn it on again after the experiment](https://docs.oracle.com/cd/E37670_01/E36387/html/ol_aslr_sec.html).

-----
### Useful material:

- https://turkeyland.net/projects/overflow/crash.php
- http://www.cse.scu.edu/~tschwarz/coen152_05/Lectures/BufferOverflow.html
- https://www.tutorialspoint.com/cprogramming/c_data_types.htm
- http://www.asciitable.com/
- https://www.cs.umd.edu/class/sum2003/cmsc311/Notes/Mips/stack.html

..and
* http://www.programmerinterview.com/index.php/data-structures/difference-between-stack-and-heap/
* https://www.youtube.com/watch?v=_8-ht2AKyH4
* https://www.youtube.com/watch?v=xDVC3wKjS64