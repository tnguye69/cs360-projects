/*******************************************************************
 * Tyler Nguyen
 * cs360 - LAB 4
 * tarc.c
 * Description: tarc.c takes a single argument on the 
 				commandline(directory) and creates a tarfile, which
				has enough information to recreate the directory
				and all of it's contents.
*******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include "dllist.h"
#include "jrb.h"


/*************************************************/
				/* PROCEDURES */
/*************************************************/

/* Procedure to partition directories and files into seperate Dllists */
void process_directory(char *fileName, Dllist directories, Dllist files){
	DIR *dir;
	struct dirent *de;
	struct stat buffer;
	int exists;
	char *s;
	Dllist traversal,tmp;      // traversal dllist is used for recursion
	traversal = new_dllist();

	/* Open directory for reading */
	dir = opendir(fileName);
	if(dir == NULL){
		fprintf(stderr,"%s is not a directory\n", fileName);
		exit(1);
	}

	/* use string s to store the file names that are directory/filename */
	s = (char*)malloc(sizeof(char)*strlen(fileName)+258); // maximum file length is 256 bytes

	/* read each filename in current directory */
	for(de = readdir(dir); de != NULL; de = readdir(dir)){
		sprintf(s, "%s/%s", fileName, de->d_name);  // construct s (directory/filename)
		exists = lstat(s, &buffer);   // call lstat on file 
		if(exists<0){
			fprintf(stderr, "Couldn't stat %s\n", s);
		}

		/* partition files into directories or files */
		else{ 
			/* if file is a directory and not a "." or ".." */
			if(de->d_type==DT_DIR && strcmp(de->d_name, ".")!=0 && strcmp(de->d_name, "..")!=0){
				dll_append(directories, new_jval_s(strdup(s)));
				dll_append(traversal, new_jval_s(strdup(s)));  // also add to traversal for recursion
			}
			/* file is regular file */
			else if(de->d_type==DT_REG){
				dll_append(files, new_jval_s(strdup(s)));
			}
		}
	}

	/* close directory and then make recursive calls to all other directories */
	closedir(dir);
	dll_traverse(tmp, traversal){
		process_directory(tmp->val.s, directories, files);
		free(tmp->val.s);
	}

	/* free memory */
	free_dllist(traversal);
	free(s);
}

/* Procedure to return a four-byte integer in little endian */
void get_four_byte_int(int *bytes, int integer){
	bytes[3] = (integer >> 24) & 0xFF;   // convert integer into little endian
	bytes[2] = (integer >> 16) & 0xFF;
	bytes[1] = (integer >> 8) & 0xFF;
	bytes[0] = integer & 0xFF;
}

/* Procedure to return an eight-byte long in little endian */
void get_eight_byte_long(int *bytes, long integer){
	bytes[7] = (integer >> 56) & 0xFF;
	bytes[6] = (integer >> 48) & 0xFF;
	bytes[5] = (integer >> 40) & 0xFF;
	bytes[4] = (integer >> 32) & 0xFF;
	bytes[3] = (integer >> 24) & 0xFF;
	bytes[2] = (integer >> 16) & 0xFF;
	bytes[1] = (integer >> 8) & 0xFF;
	bytes[0] = integer & 0xFF;	
}

/* Procedure to check if there is a prefix in directory call */
/* if '/' found, then return amount of characters up to the last '/' */
/* else it returns -1 */
int prefixCheck(char* command){
	int ret, len, i;
	len = strlen(command);
	for(i = len-1; i>=0; i--){
		// check if there are any slashes
		if(command[i] == '/'){
			ret = i;
			return ret;   // return amount of characters up to '/'
		}
	}	
	ret = -1;
	return ret;  // if none found, then return -1	
}

/* Procedure to remove the prefix in the string */
char* remove_prefix(char *string, int count){
	int i;
	int len = strlen(string);
	char *retString = (char*)malloc(sizeof(char)*len);  // allocate mem for return string
	for(i = 0; i < len-count; i++){
		retString[i] = string[count+i+1];  // return the suffix only
	}	

	return retString;
}

/* Procedure to print directory */
void print_directory(char *fileName, struct stat buffer, JRB fileTree){
	int fourByteInt[4];
	int eightByteLong[8];
	int i, fileNameSize, fileMode;
	long inode, fileModTime;

	/* FILE NAME SIZE */
	fileNameSize = (int)strlen(fileName);
	get_four_byte_int(fourByteInt, fileNameSize);
	for(i=0; i<4; i++){
		fputc(fourByteInt[i], stdout);
	}
	/* FILE NAME */
	for(i=0; i<fileNameSize; i++){
		fputc(fileName[i], stdout);
	}
	/* FILE INODE */
	inode = (long)buffer.st_ino;
	get_eight_byte_long(eightByteLong, inode);
	for(i=0; i<8; i++){
		fputc(eightByteLong[i], stdout);
	}

	/* check if we encountered this inode before */
	// if not, add to tree and print mode and modTime
	if(jrb_find_int(fileTree, inode) == NULL){
		jrb_insert_int(fileTree, inode, new_jval_i(1)); // add node to tree

		/* FILE MODE */
		fileMode = (int)buffer.st_mode;
		get_four_byte_int(fourByteInt, fileMode);
		for(i=0; i<4; i++){
			fputc(fourByteInt[i], stdout);
		}

		/* FILE MOD TIME */
		fileModTime = (long)buffer.st_mtime;
		get_eight_byte_long(eightByteLong, fileModTime);
		for(i=0; i<8; i++){
			fputc(eightByteLong[i], stdout);
		}
	}
}

/* Procedure to print file */
void print_file(char* fileName, struct stat buffer, JRB fileTree, FILE *fp){
	int fourByteInt[4];
	int eightByteLong[8];
	int i, fileNameSize, fileMode;
	long inode, fileModTime, fileSize;

	/* FILE NAME SIZE */
	fileNameSize = (int)strlen(fileName);
	get_four_byte_int(fourByteInt, fileNameSize);  // print file name size
	for(i=0; i<4; i++){
		fputc(fourByteInt[i], stdout);
	}
	/* FILE NAME */
	for(i=0; i<fileNameSize; i++){
		fputc(fileName[i], stdout);
	}
	/* FILE INODE */
	inode = (long)buffer.st_ino;   // print file's inode
	get_eight_byte_long(eightByteLong, inode);
	for(i=0; i<8; i++){
		fputc(eightByteLong[i], stdout);
	}

	/* check if we encountered this inode before */
	// if not, add to tree and print mode,modTime, fileSize and file Bytes 
	if(jrb_find_int(fileTree, inode) == NULL){
		jrb_insert_int(fileTree, inode, new_jval_i(1));  // add node 

		/* FILE MODE */
		fileMode = (int)buffer.st_mode;
		get_four_byte_int(fourByteInt, fileMode);
		for(i=0; i<4; i++){
			fputc(fourByteInt[i], stdout);
		}

		/* FILE MOD TIME */
		fileModTime = (long)buffer.st_mtime;
		get_eight_byte_long(eightByteLong, fileModTime);
		for(i=0; i<8; i++){
			fputc(eightByteLong[i], stdout);
		}

		/* FILE SIZE */
		fileSize = (long)buffer.st_size;
		get_eight_byte_long(eightByteLong, fileSize);
		for(i=0; i<8; i++){
			fputc(eightByteLong[i], stdout);
		}	

		/* THE BYTES */
		int c;
		while(1){
			c = fgetc(fp);
			if( feof(fp) ){
				break;
			}
			fputc(c, stdout);
		}
	}
}


/*************************************/
		/* MAIN FUNCTION */
/*************************************/
int main(int argc, char ** argv){
	int prefixFlag;
	struct stat buffer;
	char *fileName;
	FILE *fp;
	JRB fileTree;  // tree used for holding duplicate inodes
	Dllist directories, files, tmp;

	/* make tree and directory */
	directories = new_dllist();
	files = new_dllist();
	fileTree = make_jrb();

	/* check if there is a pathname with '/' character in it */
	char *command = (char*)malloc(sizeof(char)*(strlen(argv[1])));  // free
	strcpy(command, argv[1]);
	prefixFlag = prefixCheck(command);  // prefixFlag is either -1 or num of chars up to last slash

	/* partition directories and files into seperate dllists */
	dll_append(directories, new_jval_s(argv[1]));
	process_directory(argv[1], directories, files);

	/* traverse through directories and print */
	dll_traverse(tmp, directories){
		lstat(tmp->val.s, &buffer);  // call stat on file

		/* check if prefix needs to be cut */
		if(prefixFlag != -1){  // cut prefix
			fileName =	remove_prefix(tmp->val.s, prefixFlag);
		}
		else{  // no need to cut prefix
			fileName = tmp->val.s;
		}

		print_directory(fileName, buffer, fileTree);  // print
	}
	/* traverse through files and print */
	dll_traverse(tmp, files){
		lstat(tmp->val.s, &buffer);  // call stat on file

		/* check if prefix needs to be cut */
		if(prefixFlag != -1){  // cut prefix
			fileName =  remove_prefix(tmp->val.s, prefixFlag);
		}
		else{  // no need to cut prefix
			fileName = tmp->val.s;
		}

		/* open file to read the bytes and error check */
		fp = fopen(tmp->val.s, "rb");
		if(fp == NULL){
			perror("Error: ");
			exit(1);
		}
		print_file(fileName, buffer, fileTree, fp);  // print
		fclose(fp);  // close file
	}

	/* free memory */
	free_dllist(files);
	free_dllist(directories);
	jrb_free_tree(fileTree);
	free(command);

	return 0;
}
