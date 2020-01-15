/*************************************************************
 * Tyler Nguyen
 * cs360 - lab 7
 * jsh2.c 
 * Overview: Jsh2 is a very primitive c-interpreter shell that 
			 will let you execute commands and redirect their
             input/output. Jsh2 will allow you to use the 
             operations <, >, >>, &. Jsh2 will implement input
			 and output redirection to files.
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

	int i, status,j;
	int ampCheck = 0;
	int dirCheck = 0;
	IS is = new_inputstruct(NULL);
	char *path;
	char **newargv;
	char *argv1 = argv[1];
	char *input = NULL;
	char *output = NULL;
	char *appendOutput = NULL;
	JRB ampTree = make_jrb();
	int readFile, writeFile, appendFile;

	/* start jsh */
	if(argv[1]==NULL) { printf("jsh: "); } // display jsh: prompt if argv = NULL
	else if(strcmp(argv1, "-")!=0){ printf("%s: ", argv1); }  // print prompt

	/* begin reading input */
	while(get_line(is) >= 0){
		if(argv[1]==NULL) { printf("jsh: "); }  // display jsh: prompt if argv = NULL
		else if(strcmp(argv1, "-")!=0){ printf("%s: ", argv1); }  // pfing prompt

		ampCheck = 0;   // reset ampCheck
		readFile = -1;  // reset file descriptors
		writeFile = -1;
		appendFile = -1;

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

				j = 0;  // j is used for counting in piping
				dirCheck = 0; // reset dirCheck (flag)

				/* check for file redirection(< , > , >>, |), if we have, then open the file */
				for(i=0; i<is->NF; i++){
					if(newargv[i] == NULL) break;
					/* if < */
					if(strcmp(newargv[i], "<")==0){
						input = newargv[i+1];
						readFile = open(input, O_RDONLY);
						dirCheck = 1;
						newargv[i] = NULL;  // set arrow and string after to null
						i++;
						newargv[i] = NULL;
					}
					/* if > */
					else if(strcmp(newargv[i], ">")==0){
						output = newargv[i+1];
						writeFile = open(output, O_WRONLY | O_TRUNC | O_CREAT, 0644);
						dirCheck = 1;
						newargv[i] = NULL;  // set arrow and string after to null
					}
					/* if >> */
					else if(strcmp(newargv[i], ">>")==0){
						appendOutput = newargv[i+1];	
						writeFile = open(appendOutput, O_WRONLY | O_APPEND | O_CREAT, 0644);
						dirCheck = 1;
						newargv[i] = NULL;  // set arrow and string after to null
						i++;
						newargv[i] = NULL;
					}
					else{
						newargv[j] = is->fields[i];  // keep adding words newargv 
						j++;  
					}
				}
			}

			int pid, childRetVal;  // pid is used to hold original parent pid
			JRB tmp;

			/* we need to remove the last word on the command if there is a > redirection */
			if(output != NULL){
				i = 0;
				while(newargv[i] != output){
					if(newargv[i] == NULL) break;
					else if(newargv[i] == output) newargv[i] = NULL;
					i++;
				}
				newargv[i] = NULL;
			}

			/* fork the processes and call commands */
			if(ampCheck==0){
				// fork the process and call the command
				pid = fork();
				if(pid==0){
					/* if we have a redirection, open files for input/output */
					if(dirCheck == 1){
						if(readFile != -1) dup2(readFile, 0);
						close(readFile);

						if(writeFile != -1) dup2(writeFile,1);
						close(writeFile);
					}

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

					/* redirection commands */
					if(dirCheck == 1){
						if(readFile != -1) dup2(readFile, 0);
						close(readFile);

						if(writeFile != -1) dup2(writeFile,1);
						close(writeFile);
					}

					/* call command */
					execvp(newargv[0],newargv);
					perror(newargv[0]);
					exit(1);
				}
			}
		}
		/* reset everything before next line */
		input = NULL;
		output = NULL;
		appendOutput = NULL;
		close(readFile);
		close(writeFile);
		close(appendFile);
	}	

	jettison_inputstruct(is);
	return 0;
}
