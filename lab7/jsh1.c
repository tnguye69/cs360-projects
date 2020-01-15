/*************************************************************
 * Tyler Nguyen
 * cs360 - lab 7
 * jsh1.c 
 * Overview: Jsh1 is a very primitive c-interpreter shell that 
			 will let you execute commands and redirect their
             input/output. Jsh1 is a very bare bones shell with
            no frills. Meaning jsh1 only interprets the &.
 *************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "fields.h"
#include "jrb.h"

/* main function */
int main(int argc, char ** argv){

	if(argc > 2) { fprintf(stderr, "usage: jsh3 [prompt]\n"); exit(1); }

	int i, status;
	int ampCheck = 0;
	IS is = new_inputstruct(NULL);
	char *path;
	char **newargv;
	char *argv1 = argv[1];
	JRB ampTree = make_jrb();

	/* start jsh */
	if(argv[1]==NULL) { printf("jsh: "); } // display jsh: prompt if argv = NULL
	else if(strcmp(argv1, "-")!=0){ printf("%s: ", argv1); }  // print prompt

	/* begin reading input */
	while(get_line(is) >= 0){
		if(argv[1]==NULL) { printf("jsh: "); }  // display jsh: prompt if argv = NULL
		else if(strcmp(argv1, "-")!=0){ printf("%s: ", argv1); }  // pfing prompt

		ampCheck = 0;   // reset ampCheck

		/* if there is a command, check if it's valid */
		if(is->NF > 0){
			if(is->NF==1){  // there's only 1 word command
				path = is->fields[0];
				newargv = (char**) malloc(sizeof(char*)*2);
				newargv[0] = path;
				newargv[1] = NULL;
			}
			else{  // there's more than 1 word in the command
				path = is->fields[0];
				/* if command line ends with an ampersand */
				if(strcmp(is->fields[is->NF-1], "&")==0){
					ampCheck = 1;
					newargv = (char**) malloc(sizeof(char*)*is->NF);
					for(i=0; i<is->NF-1; i++){
						newargv[i] = is->fields[i];
					}
					newargv[is->NF-1] = NULL;
				}
				else{ // if no ampersand
					newargv = (char**) malloc(sizeof(char*)*(is->NF+1));
					for(i=0; i<is->NF; i++){
						newargv[i] = is->fields[i];
					}
					newargv[is->NF] = NULL;
				}
			}
		}

		int pid, childRetVal;  // pid is used to hold original parent pid
		JRB tmp;


		/* fork the processes and call commands */
		if(ampCheck==0){
			// fork the process and call the command
			pid = fork();
			if(pid==0){

				/* call command */
				execvp(newargv[0], newargv);
				perror(newargv[0]);
				exit(1);
			}
			else{  // if there was no ampersand, then wait
				/* keep waiting until you come accross the pid you forked with */
				childRetVal = wait(&status);
				while(childRetVal != pid){
					tmp = jrb_find_int(ampTree, childRetVal);
					if(tmp!=NULL){
						jrb_delete_node(tmp);  // delete ampersand child from tree (zombie process)
					}
					childRetVal = wait(&status);
				}
			}
		}
		else{ // there's an ampersand
			pid = fork();
			if(pid==0){
				jrb_insert_int(ampTree, pid, new_jval_i(1));  // add ampersand node into tree keyed on pid

				/* call command */
				execvp(newargv[0],newargv);
				perror(newargv[0]);
				exit(1);
			}
		}
}	

jettison_inputstruct(is);
return 0;
}
