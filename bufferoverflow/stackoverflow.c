#include <stdio.h>
#include <string.h>
#include <ctype.h>

/*
 * This code is based on two buffer overflow examples,
 * founded here: https://turkeyland.net/projects/overflow/crash.php
 * and here: http://www.cse.scu.edu/~tschwarz/coen152_05/Lectures/BufferOverflow.html .
 * And, basically, there is nothing more to understand about stack overflow to play with this.
 * But you should know about:
 *  C data types: https://www.tutorialspoint.com/cprogramming/c_data_types.htm ;
 *  and ASCII: http://www.asciitable.com/ . (Because memory addresses can be bad sometimes).
 *
 * 2017 - Alexandre R. Gomes. agomes@lasca.ic.unicamp.br
*/

void walk(void *a, int bytes)
{
  int i;
  int j;
  char *addr = a;

  /* Make sure address is 8-byte aligned */
  while (((long)addr) % 8) addr++;

  for (i = 0; i < bytes / 8; i++)
  {
    printf("%p: ", addr);

    /* Print hex values */
    for (j = 0; j < 8; j++) {
      printf("%02hhx", addr[j]);
      /* We want to play with function address,
       * so 4 bytes blocks are a good view */
      if (j%4==3) printf(" ");
    }

    /* Print ascii values */
    for (j = 0; j < 8; j++)
      printf("%c", isprint(addr[j]) ? addr[j] : '?');

    addr -= 8;
    printf("\n");
  }
  return;
}

void hello(const char* input) {
  int local = 0xefbeadde;
  char buf[8];

  /* each +1 counts 4 bytes (read the links in header),
   * each two gives one line above (because we print 8 bytes)
   * and yes, that is enough to see the RETURN ADDR (our point here).
   * 40 bytes gives us 5 lines. (Yes, I see that repeated use of values,
   * but dont use variables to store this, it will increase
   * your stack memory and mess up..)
  */
  walk(&local+3, 40);

  /* I tryied with scanf(), does not work properly (at least until I tried)
   * and it is a good point to know why. Seacrh for differences in how the data
   * come up with scanf() and strcpy().
  */
  strcpy(buf, input);
  printf("\nYou entered: '%s'\n\n", buf);

  walk(&local+3, 40);
  printf("%04hhx\n", local);
}

/* That is the goal */
void hidden(){
	printf("\n[!] Correct overflow.\n");
}

int main(int argc, char *argv[], char *envp[]) {  
  printf("Address of main is   %p\n", main);
  printf("Address of hidden is %p\n\n", hidden); // And that is the shortcut
  hello(argv[1]);
  
  printf("[!] Returned to main.\n"); // Dont fall here
  return 0;
}
