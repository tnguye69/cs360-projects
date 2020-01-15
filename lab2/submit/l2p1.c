/***************************************************************
 * Tyler Nguyen
 * cs360 - LAB 2
 * l2p1.c
 * This program reads information from the file converted and 
   printed out the IP address, and all names for machine. 
   This program strictly uses buffered I/O routines.
 ***************************************************************/
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include "jrb.h"
#include "dllist.h"
#include "fields.h"

/* Val struct used for RB tree */
typedef struct {
	unsigned char address[4];
	JRB names;
} Val;


/* INT MAIN */
main(int argc, char **argv)
{
	FILE *file;
	int i, j, k, nread, numberOfNames, count;
	long bytes;
	char *curName, *localName, *str;
	JRB tree, nameNode, tmp;
	Val *valStruct;

	/* open the file */
	file = fopen("converted", "r");
	if (file == NULL) { perror(argv[1]); exit(1); }

	/* get the number of bytes within file */
	for(bytes = 0; fgetc(file) != EOF; bytes++);
	unsigned char buffer[bytes];   // declare buffer array	 
	rewind(file);                  // rewind the file back to the beginning

	/* read the file into buffer */
	fread(buffer, 1, bytes, file);

	/* read in the contents of CONVERTED */
	i = 0;
	tree = make_jrb();   // make tree
	while(i < bytes) {
		/* CREATE VAL STRUCT */
		valStruct = malloc(sizeof(Val));
		valStruct->names = make_jrb();  // create new tree for names

		/* IP ADDRESS */
		for (j = 0; j < 4; j++){
			valStruct->address[j] = buffer[i+j];
		}
		i = i+4;                  // increment to next 4 bytes

		/* NUMBER OF NAMES */
		numberOfNames = 0;
		for(j = 0; j < 4; j++){
			numberOfNames += (int)buffer[i+j];
		}
		i = i+4;  // increment to next 4 bytes

		/* NAMES */
		while(numberOfNames != 0){
			count = 0;
			j = i;   // start j at index i
			while(buffer[j] != '\0'){   // read until the end of name
				count++;
				j++;
			}
			curName = (char*)malloc(sizeof(char)*count);  // allocate memory for curName
			for(j = 0; j < count; j++){      // copy name into curName
				curName[j] = buffer[i+j];
			}
			jrb_insert_str(valStruct->names, curName, new_jval_v(NULL));  // insert name into names struct

			// check for PERIODS in curName, in order to get localName
			localName = (char*)malloc(sizeof(char)*count);  // create copy of curName
			strcpy(localName, curName);	
			localName = strtok(localName, ".");
			if(strcmp(localName, curName) != 0){  // add localName to struct if period was found
				jrb_insert_str(valStruct->names, localName, new_jval_v(NULL));
			}

			numberOfNames = numberOfNames-1;  // decrement numOfNames
			i = i+count+1;   // increment i	
		}

		/* once names(struct) tree is loaded, traverse through and add to main tree */
		/*keyed on name, and valued on struct*/
		jrb_traverse(tmp, valStruct->names){
			jrb_insert_str(tree, jval_s(tmp->key), new_jval_v((void*)valStruct));
		}
	}

	char input[100];   // for user input
	JRB dupNameTree;
	int scanfReturn;
	/* GET USER INPUT */
	printf("Hosts all read in\n\n");
	j = 0;
	while(j != 1){
		printf("Enter host name: ");
		scanfReturn = scanf("%s", input);   // get user string input
		if(scanfReturn == EOF){  // end of input
			break;
		}
		else if(scanfReturn == 1){
			tmp = jrb_find_str(tree, input);
			if(tmp != NULL){    // machine name has been found
				dupNameTree = make_jrb();
				jrb_traverse(tmp, tree){
					if(strcmp(input,tmp->key.s)==0){  // add duplicate input to a new tree
						valStruct = (Val*)tmp->val.v;
						jrb_traverse(nameNode, valStruct->names){  // add first name of struct to get alphebetical order
							jrb_insert_str(dupNameTree, nameNode->key.s, new_jval_v((void*)valStruct));
							break;
						}
					}
				}
				//traverse through nameDupTree and print
				jrb_traverse(tmp, dupNameTree){
					valStruct = (Val*)tmp->val.v;
					for(i = 0; i < 3; i++){   // print IP address
						printf("%u.", valStruct->address[i]);
					}
					printf("%u: ", valStruct->address[3]);
					jrb_traverse(nameNode, valStruct->names){  // print names
						printf("%s ", nameNode->key.s);
					}
					printf("\n\n");
				}
				free(dupNameTree);   // free tree for next use
			}
			else{   // no key has been found
				if(input[0] == 3){  // if end of file is found, break!
					break;
				}
				printf("no key %s\n\n", input);
			}
		}
	}

	/* free memory */
	jrb_free_tree(tree);
	fclose(file);
	exit(0);
}
