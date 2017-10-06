/*
/*
 * bruteforce.c
 * 
 * Copyright (c) 2017 A Gomes <agomes@lasca.ic.unicamp.br>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * bruteforce -l <wordlist> -f <file.zip>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


//#define DEBUG 1
#define CHECK_CODE 1009
#define MAX_PWD_LEN 127


void usage_error(char* program) {
	fprintf(stderr, "Usage: %s -f file.zip -l wordlist.txt\n", program);
	exit(1);
}


int check_success(FILE *response){
	if (response == NULL) {
		return 0;
	}
	char skipping_token[9]	= "skipping:";
	char error_token[18]	= "incorrect password";
	char success_token[10] 	= "extracting";
	int s=0, e=0, k=0, success_flag=0;
	char c;
	
	while( (c = getc(response)) != EOF) {
		//printf("%c", c);	// HEAVY debug
		
		
		if (c == skipping_token[s]) {
			s++;
			if (s == 9) return 0;
		} else {
			s = 0;
		}
		
		if (c == error_token[e]) {
			e++;
			if (e == 18) return 0;
		} else {
			e = 0;
		}
		
		if (c == success_token[k]) {
			k++;
			if (k == 10) success_flag = 1;
		} else {
			k = 0;
		}
	}
	
	/*
	 * Only return 1 if success_token was found AND
	 * (NO skipping OR error) tokens was present.
	 * 
	 */
	//printf("Success flag[%d]\n", success_flag);
	if (success_flag) {
		return 1;
	} else {
		fprintf(stderr, "Unexpected zip response.\n");
		return -1;
	}
	
}


int main(int argc, char **argv)
{
	char *wordlist;
	char *zip_path;
	
	// Param check
	unsigned short check = 1000;
	int i;
	
	for (i = 1; i < argc; i++) {
		/*
		 * All parameters have two arguments (Ex: argc=5, 5-1= 4%2 ==0)
		 */
		if ((argc - i) %2 == 0) {
			/* Process non-optional arguments here. */
			if (strcmp(argv[i], "-l") == 0) {
				i++;
				wordlist = argv[i];
				check -= 1;
			} else if (strcmp(argv[i], "-f") == 0) {
				i++;
				zip_path = argv[i];
				check += 10;
			} else {
				fprintf(stderr, "Invalid parameter \"%s\".\n", argv[i]);
				usage_error(argv[0]);
			}
		} else {
			usage_error(argv[0]);
		}
	}
	if (check != CHECK_CODE) {
		usage_error(argv[0]);
	}
	
	printf("wordlist: %s.\nzip_path: %s.\n", wordlist, zip_path);
	
	
	/*
	 * Real process
	 */
	char password[MAX_PWD_LEN];
	int pass_found = 0;
	FILE *fp, *fwl;	// file_pipe, file_wordlist
	size_t len = 0;
	char resp[1024], pre_command[256], command[512];
	char tmp_pwd[MAX_PWD_LEN+1];
	fwl = fopen(wordlist, "r");
	size_t prec_len = strlen(pre_command);
	
	if (fwl == NULL) {
		fprintf(stderr, "[!] Unable to open file \"%s\".\n", wordlist);
		exit(1);
	} else {
		sprintf(pre_command, "unzip -P %%s -o %s -d /tmp/ 2>&1", zip_path);
		
		while (fgets(tmp_pwd, MAX_PWD_LEN, fwl) != NULL) {
			len = strlen(tmp_pwd);
			
			if (len > 512 - prec_len - 1) { // Another safe check
				fprintf(stderr, "Too large password found.\n");
				exit(1);
			} else if (len > 0 && strcmp(" ", tmp_pwd) !=0 ){
				strncpy(password, tmp_pwd, len-1);
				password[len-1] = '\0';
				
				//printf("tmp_pwd: %s\n", password);
				sprintf(command, pre_command, password);
				#ifdef DEBUG
				printf("CMD: %s\n", command);
				#endif
				
				fp = popen(command, "r");
				if (fp == NULL) {
					fprintf(stderr, "[!] Failed to run '%s'.\n", command);
					break;
					//fclose(fwl);
					//exit(1);
				}
				
				//printf("\tResponse: %s", resp);
				//printf("\tResponse: %s", resp);
				if (check_success(fp) == 1) {
					#ifdef DEBUG
					printf("[!] GOT!\n");
					#endif
					pass_found = 1;
					break;
				}
				
				//free(tmp_pwd);
			} else {
				fprintf(stderr, "Empty password found.\n");
				exit(1);
			}
		}
	}
	
	if (pass_found) printf("The password is %s\n", password);
	
	return 0;
}

