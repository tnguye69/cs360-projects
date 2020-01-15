/*************************************************************
 * Tyler Nguyen
 * cs360 - LAB 4
 * tarx.c
 * Description: tarx.c reads in a .tarc file and with the 
				provided information it will recreate all of 
				it's directories and files along with
				modification time and mode of each file or
				directory. This program uses system procedure
				calls such as mkdir(2v) or chmod().
*************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <utime.h>
#include "dllist.h"
#include "jrb.h"

/**********************************************/
			/* PROCEDURES */
/**********************************************/

/* Procedure to convert four bytes to integer */
int fourbyte_convert(int *bytes){
	int ret;
	ret = (bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | (bytes[0]);
	return ret;
}

/* Procedure to convert eight bytes to long */
long eightbyte_convert(long *bytes){
	long ret;
	ret = (bytes[7] << 56) | (bytes[6] << 48) | (bytes[5] << 40) | (bytes[4] << 32) 
	      | (bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | (bytes[0]);
	return ret;
}


/********************************************/
			/* MAIN FUNCTION */
/********************************************/
int main(int argc, char ** argv){
	int i, fileNameSize, mode;
	char *fileName;
	int fourBytes[4];
	long eightBytes[8];
	long inode, fileModTime, fileSize;
	JRB inodeTree = make_jrb();
	JRB directories = make_jrb();  // holds directory, valued on directory mode
	JRB dirTimes = make_jrb();  // keyed on time, valued on directory name
	JRB tmp;

	/* read in the file */
	int c;
	while((c=fgetc(stdin)) != EOF){
		/* read in the fileNameSize */
		fourBytes[0] = c;
		for(i=1; i<4; i++){
			c = fgetc(stdin);
			fourBytes[i] = c;

			if(c == EOF){  // error check
				fprintf(stderr, "Unable to read fileNameSize\n");
				exit(1);
			}
		}
		fileNameSize = fourbyte_convert(fourBytes);
		if(fileNameSize<=0){  // error check fileNameSize
			fprintf(stderr, "File name size is %d, which I can't malloc\n", fileNameSize);
			exit(1);	
		}
		else if(fileNameSize > 10000000){ // file too big
			fprintf(stderr, "file name size too big\n");
			exit(1);
		}

		/* read in fileName */
		fileName = malloc(sizeof(char) * fileNameSize * 10000); // FREE
		for(i=0; i<fileNameSize; i++){
			c = fgetc(stdin);
			fileName[i] = c;		

			if(c == EOF){  // error check
				fprintf(stderr, "File name size is %d, but bytes read = %d\n", fileNameSize, i);
				exit(1);
			}
		}

		/* read in inode */
		c = fgetc(stdin);
		eightBytes[0] = c;
		for(i=1; i<8; i++){
			c = fgetc(stdin);
			eightBytes[i] = c;

			if(c == EOF){  // error check
				fprintf(stderr, "Couldn't read inode\n");
				exit(1);
			}
		}
		inode = eightbyte_convert(eightBytes);

		/* check if file/directory(hard links) is already in tree */
		tmp = jrb_find_int(inodeTree, inode);
		// If File not added yet //
		if(tmp == NULL){
			jrb_insert_int(inodeTree, inode, new_jval_s(fileName)); // insert file/directory

			/* read in mode */
			c = fgetc(stdin);
			fourBytes[0] = c;
			for(i=1; i<4; i++){
				c = fgetc(stdin);
				fourBytes[i] = c;

				if(c == EOF){  // error check
					fprintf(stderr, "Couldn't read mode\n");
					exit(1);
				}
			}
			mode = fourbyte_convert(fourBytes);

			/* IF DIRECTORY */
			if(S_ISDIR(mode)){
				// insert directory into tree for update at end
				// keyed on fileName, valued on mode, so we can call chmod(fileName, Mode)
				jrb_insert_str(directories, fileName, new_jval_i(mode)); 

				// make directory
				mkdir(fileName, 0777);

				// read in mod time and insert mod time into tree for update
				c = fgetc(stdin);
				eightBytes[0] = c;
				for(i=1; i<8; i++){
					c = fgetc(stdin);
					eightBytes[i] = c;

					if(c == EOF){  // error check
						fprintf(stderr, "Couldn't read mtime\n");
						exit(1);
					}
				}
				fileModTime = eightbyte_convert(eightBytes);
				jrb_insert_str(dirTimes, fileName, new_jval_i(fileModTime));
			}
			/* IF FILE */
			else if(S_ISREG(mode)){
				// read in mod time
				c = fgetc(stdin);
				eightBytes[0] = c;
				for(i=1; i<8; i++){
					c = fgetc(stdin);
					eightBytes[i] = c;

					if(c == EOF){  // error check
						fprintf(stderr, "Couldn't read mtime\n");
						exit(1);
					}
				}
				fileModTime = eightbyte_convert(eightBytes);

				// read in fileSize
				c = fgetc(stdin);
				eightBytes[0] = c;
				for(i=1; i<8; i++){
					c = fgetc(stdin);
					eightBytes[i] = c;
				}		
				fileSize = eightbyte_convert(eightBytes);

				// write the files contents
				char *fileContents = (char*)malloc(fileSize+1000);  // FREE
				for(i=0; i<fileSize; i++){   // read file contents into buffer
					c = fgetc(stdin);
					fileContents[i] = c;

					if(c == EOF){ // error check
						fprintf(stderr, "Trying to read %d bytes of the file, and got EOF\n", (int)fileSize);
						exit(1);
					}
				}
				FILE *file = fopen(fileName, "wb");
				if(file == NULL){  // error check file
					perror("Error: ");
					exit(1);
				}
				for(i=0; i<fileSize; i++){  // read contents into file
					fputc(fileContents[i], file);
				}
				fclose(file);

				// change file mode
				chmod(fileName, mode);

				// change file time
				struct timeval curTime;
				gettimeofday(&curTime, NULL);
				struct timeval times[2];
				times[0].tv_sec = curTime.tv_sec;
				times[0].tv_usec = curTime.tv_usec;
				times[1].tv_sec = fileModTime;
				times[1].tv_usec = 0;
				utimes(fileName, times);
			}
		}
		// If there's a hard link //
		else{
			// link file to original inode
			link(tmp->val.s, fileName);	
		}	
	}

	/* since some directories don't have write protection, we have to update mode and time */
	jrb_traverse(tmp, directories){
		// update mode
		chmod(tmp->key.s, tmp->val.i);

		// update time
		struct timeval curTime;
		gettimeofday(&curTime, NULL);
		struct timeval times[2];
		times[0].tv_sec = curTime.tv_sec;
		times[0].tv_usec = curTime.tv_usec;
		times[1].tv_sec = jrb_find_str(dirTimes, tmp->key.s)->val.i;
		times[1].tv_usec = 0;
		utimes(tmp->key.s, times);
	}

	/* Free Memory */
	jrb_free_tree(inodeTree);
	jrb_free_tree(directories);
	jrb_free_tree(dirTimes);
	free(fileName);

	return 0;
}
