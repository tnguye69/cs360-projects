/**************************************************************************
 * Tyler Nguyen
 * lab3
 * fakemake.c
 * fakemake.c automates compiling by making one executable. This is pretty
   much a pared-down version of make(1). This program uses stat(2v) and 
   system(3) to reinforce the system calls and data. 
**************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include "fields.h"
#include "dllist.h"

/* INT MAIN */
int main(int argc, char ** argv){
	IS is;
	int i;
	Dllist tmp, cList, hList, flagList, libList;
	char *execName = NULL;
	time_t maxHeaderTime, curTime, oFileTime, cFileTime;
	struct stat buffer, oFileBuffer;
	char *oFileName;

	/* create new dllists */
	cList = new_dllist();
	hList = new_dllist();
	flagList = new_dllist();
	libList = new_dllist();
	
	/* open the file */
	if(argc != 2) { // no description file is specified 
		is = new_inputstruct("fmakefile");
	}
	else{  // read in through command line
		is = new_inputstruct(argv[1]);
		if(is == NULL){
			perror(argv[1]);
			exit(1);
		}
	}

	/* read in the description file */
	while(get_line(is) >= 0){
		if(is->NF > 0){
			/* read in executable name */
			if(strcmp(is->fields[0], "E") == 0){
				if(execName != NULL){  // check if execName has already been made
					fprintf(stderr, "fmakefile (%i) cannot have more than one E line\n", is->line);
					exit(1);
				}
				else{  // malloc and copy name 
					execName = (char*)malloc(sizeof(char)*sizeof(is->fields[1]));  // make sure to free
					strcpy(execName, is->fields[1]); 
				}
			}	
			/* C lines */
			else if(strcmp(is->fields[0], "C") == 0){
				//add C files into dllist
				for(i = 1; i < is->NF; i++){
					dll_append(cList, new_jval_s(strdup(is->fields[i])));
				}
			}
			/* H lines */
			else if(strcmp(is->fields[0], "H") == 0){
				//add H files into dllist
				for(i = 1; i < is->NF; i++){
					dll_append(hList, new_jval_s(strdup(is->fields[i])));
				}
			}
			/* L lines */
			else if(strcmp(is->fields[0], "L") == 0){
				//add lib files into dllist
				for(i = 1; i < is->NF; i++){
					dll_append(libList, new_jval_s(strdup(is->fields[i])));
				}
			}
			/* F lines */
			else if(strcmp(is->fields[0], "F") == 0){
				//add flags into dllist
				for(i = 1; i < is->NF; i++){
					dll_append(flagList, new_jval_s(strdup(is->fields[i])));
				}
			}
			/* error check for any unprocessed line */
			else{
				fprintf(stderr, "fakemake (1): Lines must start with C, H, E, F, or L\n");
				exit(1);
			}
		}
		else if(is->NF == 0){  // blank line
		}
	}

	/* flag an error if there is no E line */
	if(execName == NULL){
		fprintf(stderr, "No executable specified\n");
		exit(1);
	}
	
	/* process the header files */
	  /* RETURN THE MAX TIME */
	tmp = dll_first(hList);            // get the first header file
	if(tmp != (hList)){                // theres a header file and find the max time
		//call stat on first header file
		if(stat(jval_s(tmp->val), &buffer) == 0){
			maxHeaderTime = buffer.st_mtime;
		}
		else{   // no file found
			fprintf(stderr, "fmakefile: %s: No such file or directory\n", jval_s(tmp->val));
			exit(1);
		}

		//traverse through all the header files
		tmp = tmp->flink;
		if(tmp != (hList)){  // if there's more than one header then traverse
			dll_traverse(tmp, hList){
				if(stat(jval_s(tmp->val), &buffer) == 0){  // file found
					curTime = buffer.st_mtime;
					if(curTime > maxHeaderTime){  // if curTime is more recent than maxHeaderTime
						maxHeaderTime = curTime;
					}
				}
				else{  // no .h file found
					fprintf(stderr, "fmakefile: %s: No such file or directory\n", jval_s(tmp->val));
					exit(1);
				}
			}
		}
	}
	else{                              // no header file
		maxHeaderTime = (time_t)(-1);  // declare maxHeaderTime as NULL
	}

	/* process the C files */
	// find the .o file, and recompile certain files //
	Dllist cFileCompileList, oFileList;
	cFileCompileList = new_dllist();  // this list will be used for compiling .c files
	oFileList = new_dllist();

	dll_traverse(tmp, cList){
		if(stat(jval_s(tmp->val), &buffer) != 0){  // .c file doesn't exist
			// do nothing, we will flag error later
		}
		else{   // .c file exists
			// retrieve .o file name
			oFileName = (char*)malloc(sizeof(char)*sizeof(jval_s(tmp->val))); 
			strcpy(oFileName, jval_s(tmp->val));
			oFileName = strtok(oFileName, ".");   // retreive string before period
			strcat(oFileName, ".o");              // add .o to oFileName
			dll_append(oFileList, new_jval_s(oFileName));   // add all .o files to oFileList	

			// look for the .o file
			if(stat(oFileName, &oFileBuffer) != 0){  // if .o file doesn't exist then recompile .c file
				dll_append(cFileCompileList, tmp->val);
			}
			else{  // if .o file exists, then compare times and see if we need to recompile
				oFileTime = oFileBuffer.st_mtime;  // retreive oFileTime
				cFileTime = buffer.st_mtime;       // retreive cFileTime
				
				// if oFileTime is less recent than cFileTime OR maxHeaderTime, then recompile .c file
				if((oFileTime<cFileTime) || (oFileTime<maxHeaderTime)){
					dll_append(cFileCompileList, tmp->val);
				}
			}
		}
	}

	/* remake .c files */
	char *compileC = (char*)malloc(sizeof(char)*100);
	char *printCompileC = (char*)malloc(sizeof(char)*100);
	strcpy(compileC, "gcc -c ");  // command for compiling .c files
	dll_traverse(tmp, flagList){  // add flags into compileC string
		strcat(compileC, jval_s(tmp->val));		
		strcat(compileC, " ");
	}
	dll_traverse(tmp, cFileCompileList){  // call system on every .c file that needs to be compiled
		strcpy(printCompileC, compileC);
		strcat(printCompileC, jval_s(tmp->val));  // add .c files to gcc -c command
		printf("%s\n", printCompileC);  // actual print command used for system
		if(system(printCompileC) != 0){ // call system on printCompileC
			fprintf(stderr, "Command failed.  Exiting\n");
			exit(1);
		}
	}
	dll_traverse(tmp, cList){  // flag any errors if any .c files don't exist
		if(stat(jval_s(tmp->val), &buffer) != 0){
			fprintf(stderr, "fmakefile: %s: No such file or directory\n", jval_s(tmp->val));
			exit(1);
		}
	}

	/* check if any files were remade, if not, get maximum time of oFiles */
	int printExec = 0;  // if printExec = 0, then don't print
	time_t oTimeMax;
	if(dll_empty(cFileCompileList) == 0){  // if list is not empty, then print exec
		printExec = 1;
	}
	else{  // cFileList is empty so find oTimeMax of .o files
		tmp = dll_first(oFileList);  // obtain first node in oFileList
		if(tmp != (oFileList)){      // if oFileList is not empty
			//call stat on first node and get time
			if(stat(jval_s(tmp->val), &buffer) == 0){
				oTimeMax = buffer.st_mtime;
			}
			
			//traverse through all of the .o files and find max time
			tmp = tmp->flink;
			if(tmp != (oFileList)){
				dll_traverse(tmp, oFileList){
					if(stat(jval_s(tmp->val), &buffer) == 0){
						curTime = buffer.st_mtime;
						if(curTime > oTimeMax){
							oTimeMax = curTime;
						}
					}
				}
			}
		}
	}

	/* check if EXECUTABLE needs to be printed */
	  /* if executable exists, and is less recent than .o files, print */   
	if(stat(execName, &buffer) == 0){  // executable exists
		curTime = buffer.st_mtime;  // curTime is exec time
		if(curTime < oTimeMax){
			printExec = 1;
		}
	}
	else{  // executable doesn't exist so print
		printExec = 1;
	}

	/* print EXECUTABLE */
	char* compileExecFile = (char*)malloc(sizeof(char)*100);
	strcpy(compileExecFile, "gcc -o ");
	strcat(compileExecFile, execName);  // add execName
	dll_traverse(tmp, flagList){  // add flags
		strcat(compileExecFile, " ");
		strcat(compileExecFile, jval_s(tmp->val));
	}
	dll_traverse(tmp, oFileList){  // add .o files
		strcat(compileExecFile, " ");
		strcat(compileExecFile, jval_s(tmp->val));
	}
	dll_traverse(tmp, libList){  // add lib files
		strcat(compileExecFile, " ");
		strcat(compileExecFile, jval_s(tmp->val));
	}

	/* call system on compileExecFile */
	if(printExec == 1){
		printf("%s\n", compileExecFile);
		if(system(compileExecFile) != 0){  // error check system call
			fprintf(stderr, "Command failed.  Fakemake exiting\n");
			exit(1);
		}
	}
	else{ // f is up to date
		printf("%s up to date\n", execName);
		exit(1);
	}
	

	/* free memory */
	free_dllist(cList);
	free_dllist(hList);
	free_dllist(libList);
	free_dllist(flagList);
	free_dllist(cFileCompileList);
	free_dllist(oFileList);
	free(execName);
	free(oFileName);
	free(compileC);
	free(printCompileC);
	free(compileExecFile);

	jettison_inputstruct(is);
	return 0;
}
