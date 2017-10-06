/*
 * wordharvest.c
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
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include <unistd.h>

#define CHECK_CODE 1009
#define SIZE 4096
#define BOOK_SIZE 512
#define PWD_BUFFER_SIZE 127
//#define DEBUG 1
//#define VERBOSE 1


/*
 * book[i] = page		book	= **char
 * page[j] = word		page	= *char
 * word[k] = letter		letter	= char
 */
FILE *fout;
char *wlist[SIZE];
char **book[BOOK_SIZE];
char c = ' ';
char tmp_word[128];
short wl_size = 0, b_pages=0;


void usage_error(char* program) {
	fprintf(stderr, "Usage: %s -d directory/ -o output/file [-e ext1:ext2]\n", program);
	exit(1);
}


void safe_exit() {
	//printf("[!] Entering safe exit.\n");
	
	int i, j;
	
	for (i=0; i < wl_size; i++) {
		free(wlist[i]);
	}
	
	//printf("[!] Releasing book.\n");
	
	for (i=0; i < b_pages; i++) {
		// For each page
		char **page = book[i];
		
		for (j=0; j<SIZE; j++) {
			#ifdef DEBUG
			printf("Book[%d]. Word: %s.\n", i, page[j]);
			#endif
			free(page[j]);
		}
		free(book[i]);
	}
	
	fclose(fout);
	
	#ifdef VERBOSE
	printf("[!] Safe exit done.\n");
	#endif
	
	// Random information print
	//printf("\n[!] Size: %d.\n", (int) sizeof(char***));
}


void initialize_wlist() {
	int i;
	
	for (i=0; i<SIZE; i++) {
		wlist[i] = NULL;
	}
}


void print_list() {
	int i;
	
	printf("##############################\n# Word List:");
	for (i=0; i<wl_size; i++) {
		if (i % 5 ==0) {
			printf("\n# %s ", wlist[i]);
		} else {
			printf("%s ", wlist[i]);
		}
	}
	printf("\n##############################\n");
}


int cmpfunc(const void *a, const void *b) {
	const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    /* strcmp functions works exactly as expected from comparison
     * function
     */ 
    
    return strcmp(*ia, *ib);
}


int create_book_page() {
	if (b_pages == BOOK_SIZE) {
		printf("[!] FATAL ERROR: BOOK_SIZE insufficient!\n");
		
		return -1;
	}
	
	printf("[!] Creating new book page n %d. (%d entries, %d bytes) \n", b_pages, SIZE, (int) sizeof(wlist));
	char **page = (char**) malloc(sizeof(wlist) );
	
	book[b_pages++] = page;
	#ifdef DEBUG
	print_list();
	#endif
	qsort(wlist, SIZE, sizeof(char *), cmpfunc);
	#ifdef VERBOSE
	print_list();
	#endif
	int j;
	for (j=0; j<SIZE; j++) {
		page[j] = wlist[j];
	}
	
	//printf("TEST: %s...%s\n", page[0], page[SIZE-1]);
	// restart word list
	wl_size = 0;
	// memset(wlist, 0, sizeof wlist);
	
	return 0;
}


int count_word(char* word) {
	int i;
	char** item;
	
	for (i=0; i<b_pages; i++) {
		#ifdef DEBUG
		printf("\tStack: %p\n", &wlist);
		printf("\tpage : %p (%s)\n", book[i], (char*) *book[i]);
		printf("Looking at Book page %d.\n", i);
		#endif
		
		item = (char**) bsearch (&word, book[i], SIZE, sizeof(char *), cmpfunc);
		if( item != NULL ) {
			#ifdef DEBUG
			printf("Found item = %s.\n", *item);
			#endif
			
			free(word);
			return 1;
		}
	}
	
	for (i=0; i<wl_size; i++) {
		#ifdef DEBUG //-------------------------------------------------
		printf("Comparing: [%s] with [%s].\n", word, wlist[i]);
		#endif //-------------------------------------------------------
		
		if (strcmp(word, wlist[i]) == 0) {
			//printf("Already used.\n");
			free(word);
			return 1;
		}
	}
	#ifdef VERBOSE //---------------------------------------------------
	printf("Adding [%s] to list.\n", word);
	#endif //-----------------------------------------------------------
	
	// add to list and write to file
	wlist[wl_size++] = word;
	
	if (wl_size == SIZE ) {
		if (create_book_page() < 0){
			return -1;
		}
	}
	
	// TODO: File
	if (fout) {
		fputs(word, fout);
		fputs("\n", fout);
		
		return 0;
	} else {
		return -1;
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
/* =====================================================================
 * =====================================================================
 * =========================== Main
 */


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
	#ifdef VERBOSE
	printf("Searching on: %s.\nFor extensions: %s.\nOutput file: %s.\n\n", find_path, extensions, file_out);
	#endif
	
	/* Starting the real process */
	// Find files
	FILE *fp;
	char resp[1024];
	char command[256];
	char *param_regex = get_regex(extensions);
	sprintf(command, "find %s -type f %s", find_path, param_regex);
	free(param_regex);
	
	fp = popen(command, "r");
	if (fp == NULL) {
		printf("[!] Failed to run 'find'.\n");
		exit(1);
	}

	// Open output file
	fout = fopen(file_out, "a+");
	short error = 1;
	
	/* For each file returned by find command */
	while (fgets(resp, sizeof(resp)-1, fp) != NULL && error != 0) {
		char* path;
		size_t bf_len;
		path = strtok(resp, "\n");
		
		do {
			#ifdef VERBOSE
			printf("\"%s\"\n", path);
			#endif
			char buffer[PWD_BUFFER_SIZE] = {'\0'};
			
			char c;
			
			FILE *file;
			file = fopen(path, "r");
			
			if (file) {
				#ifdef DEBUG //-----------------------------------------
				printf("\n\t[!] Reading.\n\t");
				#endif //-----------------------------------------------
				while ((c = getc(file)) != EOF) {
					bf_len = strlen(buffer);
					
					#ifdef DEBUG //-------------------------------------
					printf("(%d) %s ", (int) bf_len, buffer);
					#endif //-------------------------------------------
					
					if (!isalnum(c)) {
						
						if (bf_len > 0) {
							char* new_word = (char *) malloc(sizeof(char) * (bf_len+1));
							strcpy(new_word, buffer);
							new_word[bf_len] = '\0';
							#ifdef DEBUG
							printf("[!] New word: %s\n", new_word);
							#endif
							
							if (count_word(new_word) < 0) {
								error = 0; // True
								break;
							}
							memset(buffer, 0, sizeof buffer);
						}
						// putchar(c);
					} else {
						if (bf_len < PWD_BUFFER_SIZE) {
							buffer[bf_len] = c;
						} else {
							fprintf(stderr, "[!] Too large word found!\n");
							memset(buffer, 0, sizeof buffer);
							bf_len = 0;
						}
						
					}
				}
				
				fclose(file);
				#ifdef DEBUG //-----------------------------------------
				printf("[!] Closed.\n");
				#endif //-----------------------------------------------
				//sleep(1);
			}
		} while (path = strtok(NULL, "\n"));
	}
	/* close */
	pclose(fp);
	
	/* free */
	safe_exit();
	
	return 0;
}
