This folder contains stack overflow exploration materials, including a `C` code, and instructions of how perform an illustrative attack. In this, the goal is to execute the [`hidden()`](https://github.com/gomesar/sec-stuff/blob/master/bufferoverflow/stackoverflow.c#L73) function, which was declared but not originally called.

## How to:
1. Setup

1.1 Automatically (with `sudoers` permissions):
```
$ ./compile.sh
```
1.2 Manually:

- Turn `OFF` Address Space Layout Randomization (ASLR)*
```
# echo 0 | tee /proc/sys/kernel/randomize_va_space
```
- Compile without stack protection
```
$ gcc stackoverflow.c -o exec -fno-stack-protector
```

2. Play with it:
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
