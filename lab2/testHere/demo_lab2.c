/******************************************************************
*  The purpose of this program is to illustrate the
*  use of the functions fopen(), fread(), open() and read().
*
*  To compile:  gcc demo_lab2.c
*
*  To run:  >> a.out anyfile.txt
*  NOTE #1:  anyfile.txt is any ascii text file.
*
*  Output:  The contents of the text file will be printed to
*  the screen twice (once from the fopen/fread sequence, once
*  from the open/read sequence).  If you do not want the
*  contents printed to the screen, use redirection:
*  >> a.out anyfile.txt > output
*
*  NOTE #2:  I chose to read in the file 4 bytes at a time.
*  This is because I think it will make the example more useful 
*  to you as you work on Lab2.  Under normal circumstances,
*  you might not want to read in 4 byte increments.  Why is this?
******************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>  /* be sure to include this library when using open()*/
#include<sys/types.h> /*and this one*/
#include<sys/stat.h>  /*and....guess what?!  yup, this one, too*/
                      /*in my experience, only the first one is
                        necessary, but the man pages tell us
                        to include all three.*/

int main (int argc, char **argv)
{

FILE *in;    /* use with fopen() */
int my_file; /* use with open() */

char *input;
int i=1;

/*****************************************************************/
/*  First, we'll use fopen/fread to read in the file             */
/*****************************************************************/

/*open the input file and error check*/  
in = fopen(argv[1], "r");
if (in == NULL)
{
  printf("\nError opening input file\nSyntax: >> a.out filename\n");
  exit(0);
}

/*we'll be reading in the file 4bytes at a time, so malloc 4bytes*/
input = (char*) malloc(sizeof(char) * 4);

/*read in the file using fread*/
/*if you're not sure what the arguments to fread mean, see the
  man pages (type "man fread" at a prompt) */
while (i!=0)
{
 //i = fread(input, sizeof(input), 1, in);
 i = fread(input, sizeof(input), 1, in);
 printf("%s", input);
}

/*be sure to always close the file after you are done working with it*/
fclose(in);
free (input); /*and also always free the memory that you've malloced */


/*************************************************************************/
/*  The code below does the exact same thing as the code above, but using*/ 
/*  open() and read() instead of fopen() and fread().  Note the major*/
/*  differences in syntax. */   
/*************************************************************************/


/* open the file, error check return val.  What is O_RDONLY?
   Pretty much what it sounds like--a flag that tells open()
   that we will be reading from this file*/

my_file = open(argv[1], O_RDONLY);
if(my_file < 0)
{
  printf("Error opening the file.  Syntax:  >> a.out inputfile\n");
  exit(0);
}

/*similar to above, but note the difference in syntax between
  fread and read */
input = (char*) malloc(sizeof(char) * 4);
while(read(my_file, input, sizeof(input)) > 0)
{
  //printf("%s", input);
}

close(in);
free (input);
return 0;
}
