/*
 * wordharvest.c
 * 
 * Copyright 2017 A Gomes <agomes@lasca.ic.unicamp.br>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * wordharvest [-d /tmp/] [-o words_tmp] [opt: -e txt:text:asc]
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


#define CHECK_CODE 1009
#define SIZE 2048
#define DEBUG 1

char *wlist[SIZE];
char c = ' ';
char tmp_word[128];
short int wl_size = 0;


void initialize_wlist() {
	int i;
	for (i=0; i<SIZE; i++) {
		wlist[i] = NULL;
	}
}

char* get_regex(char *ext_list){
	char *param_regex = (char *) malloc(sizeof(char) * 64);
	char *p;
	unsigned int safe = 0;
	param_regex[0] ='\0';
	
	strcat(param_regex, "-regex '.*\\.\\(");
	
	p = strtok(ext_list, ":");
	strcat(param_regex, p);
	
	#ifdef DEBUG //-----------------------------------------------------
	printf("P: \"%s\"\n", p);
	#endif //-----------------------------------------------------------
	
	while (p = strtok(NULL, ":") ) {
		#ifdef DEBUG //-------------------------------------------------
		printf("P: \"%s\"\n", p);
		#endif //-------------------------------------------------------
		
		strcat(param_regex, "\\|");
		strcat(param_regex, p);
		
		safe++;
		if (safe == 20) break;
	}
	strcat(param_regex, "\\)'");
	
	return param_regex;
}


int count_word(char* word) {
	int i;
	for (i=0; i<wl_size; i++) {
		#ifdef DEBUG //-------------------------------------------------
		printf("Comparing: [%s] with [%s].\n", word, wlist[i]);
		#endif //-------------------------------------------------------
		
		if (strcmp(word, wlist[i]) == 0) {
			printf("Already used.\n");
			return 0;
		}
	}
	#ifdef DEBUG //-----------------------------------------------------
	printf("Adding [%s] to list.\n", word);
	#endif //-----------------------------------------------------------
	
	// add to list and write to file
	wlist[wl_size++] = word;
	// TODO: File
	
	return 1;
}


/* =====================================================================
 * =====================================================================
 * =========================== Main
 */


void usage_error(char* program) {
	fprintf(stderr, "Usage: %s -d directory/ -o output/file [-e ext1:ext2]\n", program);
	exit(1);
}


int main(int argc, char **argv) {
	/* Check for correct usage */
	if (argc < 3 || argc % 2 != 1) { // Only couple n of arguments
		  usage_error(argv[0]);
	 }
	 
	 /* Conventions for command-line arguments
	  * http://courses.cms.caltech.edu/cs11/material/c/mike/misc/cmdline_args.html
	  */
	 int i;
	 char extensions[64] = "txt:text";
	 char* find_path;
	 char* file_out;
	 unsigned short check = 1000;

	 for (i = 1; i < argc; i++) {
		/*
		 * All parameters have two arguments (Ex: argc=5, 5-1= 4%2 ==0)
		 */
		if ((argc - i) %2 == 0) {
			if (strcmp(argv[i], "-e") == 0) { /* Process optional arguments. */
			       i++;
			       strcpy(extensions, argv[i]);  /* Convert string to int. */
			} else {
				/* Process non-optional arguments here. */
				if (strcmp(argv[i], "-d") == 0) {
					i++;
					find_path = argv[i];
					check -= 1;
				} else if (strcmp(argv[i], "-o") == 0) {
					i++;
					file_out = argv[i];
					check += 10;
				} else {
					fprintf(stderr, "Invalid parameter \"%s\"", argv[i]);
					usage_error(argv[0]);
				}
			}
		} else {
			usage_error(argv[0]);
		}
	}
	if (check != CHECK_CODE) {
		usage_error(argv[0]);
	}
	#ifdef DEBUG
	printf("Searching on: %s.\nFor extensions: %s.\nOutput file: %s.\n", find_path, extensions, file_out);
	#endif
	
	/* Starting the real process */
	char *param_regex = get_regex(extensions);
	// Find files
	FILE *fp;
	char path[1035];

	/* Open the command for reading. */
	char command[128];
	sprintf(command, "find %s -type f %s", find_path, param_regex);
	fp = popen(command, "r");
	if (fp == NULL) {
		printf("Failed to run command\n");
		exit(1);
	}

	/* Read the output a line at a time - output it. */
	while (fgets(path, sizeof(path)-1, fp) != NULL) {
		printf("%s", path);
	}

	/* close */
	pclose(fp);


	char test[32] = "Teste";
	count_word(test);
	
	
		// Find terms
	 
	return 0;
}
