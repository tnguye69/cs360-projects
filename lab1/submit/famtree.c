/**************************
 *Tyler Nguyen
 *Lab 1
 **************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jrb.h"
#include "dllist.h"
#include "fields.h"

/* Person Struct */
typedef struct{
	char *name;
	char sex;
	void *father;   // father will be person struct val
	void *mother;   // same as mother
	Dllist children;
	int visited;
	int printed;
} Person;

/* Function to create a Person struct */
Person *createPerson(char *name){
	Person *p;
	p = malloc(sizeof(Person));  
	p->name = strdup(name);    // initialize all values to default
	p->sex = ' ';
	p->father = NULL;
	p->mother = NULL;
	p->children = new_dllist();  // allocate new dlist
	p->visited = 0;
	p->printed = 0;
	return p;  // return person
}

/* Function to create name */
char *createName(IS is){
	char *name;
	int i, nsize;

	nsize = strlen(is->fields[1]);  // allocate memory for name
	for(i = 2; i < is->NF; i++) nsize += (strlen(is->fields[i])+1);
	name = (char*) malloc(sizeof(char)*(nsize+1));

	strcpy(name, is->fields[1]);  // copy first word into name
	nsize = strlen(is->fields[1]);  // copy in remaining words to name
	for(i = 2; i < is->NF; i++){
		name[nsize] = ' ';
		strcpy(name+nsize+1, is->fields[i]);
		nsize += strlen(name+nsize);
	}

	return name;
}

/* Function to check sex of a person */
/* 0 for exit(it's wrong), 1 for true */
int checkSex(Person *p, char sex){
	if(sex == 'M'){  // check if person is a male
		if(p->sex!='M' && p->sex!=' '){
			return 0;  // person was not a MALE
		}
		p->sex = 'M';  // declare person male if not declared
	}
	else if(sex == 'F'){  // check if person is a female
		if(p->sex != 'F' && p->sex!=' '){
			return 0;  // person was not a FEMALE	
		}
		p->sex = 'F';  // declare person famale if not declared
	}
	return 1;
}

/* Function to add child to list */
void addChild(Person *child, Dllist childList){
	int flag = 0;
	Dllist childNode;
	Person *node;

	//if list is empty, add child to list
	if(dll_empty(childList) == 1){
		dll_append(childList, new_jval_v(child));
	}
	else{  // if not check to see if child is already in list
		dll_traverse(childNode, childList){   // iterate through list
			node = (Person*) childNode->val.v;		
			if(strcmp(node->name, child->name) == 0){  // child is already in list!
				flag = 1;
			}
		}
		if(flag == 0){  // child hasn't been found, so add to list
			dll_append(childList, new_jval_v(child));
		}
	}
}

/* Function to check for cycles */
int cycleCheck(Person *p){
	Person *child;
	Dllist childNode, childList;
	childList = p->children;

	if(p->visited == 1) return 0;
	if(p->visited == 2) return 1;
	p->visited = 2;

	dll_traverse(childNode, childList){
		child = (Person*) childNode->val.v;
		if(cycleCheck(child)) return 1;
	}
	p->visited = 1;
	return 0;
}

/*create a print function*//////////////////////////////
void printTree(JRB people){
	JRB tmp;
	Person *person, *mother, *father, *child;
	Dllist node, childNode, toPrint;
	toPrint = new_dllist();

	jrb_traverse(tmp, people){  // add all people without parents to toPrint
		person = (Person*) tmp->val.v;
		if(person->mother==NULL && person->father==NULL){
			dll_append(toPrint, new_jval_v(person));
		}
	}

	while(dll_empty(toPrint) != 1){
		node = dll_first(toPrint);  // take the head node
		person = (Person*) node->val.v;
		mother = (Person*) person->mother;
		father = (Person*) person->father;

		/* if p doesn't have parents, or if p's parent's have been printed then print */
		if(person->printed == 0){  
			if((mother==NULL||father==NULL) 
					|| ((mother->printed==1||mother==NULL) && (father->printed==1||father == NULL))){
				printf("%s\n", person->name);  // NAME

				if(person->sex == 'M'){   // SEX
					printf("  Sex: Male\n");
				}
				else if(person->sex == 'F'){
					printf("  Sex: Female\n");
				}
				else{
					printf("  Sex: Unknown\n");
				}

				if(father == NULL){   // FATHER
					printf("  Father: Unknown\n");
				}
				else{
					printf("  Father: %s\n", father->name);
				}

				if(mother == NULL){   // MOTHER
					printf("  Mother: Unknown\n");
				}
				else{
					printf("  Mother: %s\n", mother->name);
				}

				if(dll_empty(person->children)==1){  // no CHILDREN
					printf("  Children: None\n");
				}
				else{   // has CHILDREN
					printf("  Children:\n");
					dll_traverse(childNode, person->children){
						child = (Person*)childNode->val.v;
						printf("    %s\n", child->name);
						dll_append(toPrint, new_jval_v(child));
						//printf("ADDING CHILD %s\n", child->name);
					}
				}
				person->printed = 1;
				printf("\n");
			}
		}
		dll_delete_node(node);
	}
	free_dllist(toPrint);
}

/************************************************************************************************
 **************MAIN FUNCTION*************************
 *************************************************************************************************/
main(int argc, char **argv){
	IS is;
	int i, nsize;
	char *name;
	Person *person, *savedPerson, *father, *mother, *child, *realFather, *realMother, *realChild;
	JRB people, tmp;
	Dllist childList, childNode;

	people = make_jrb();  //make tree

	/* input struct will read in through standard input */
	is = new_inputstruct(NULL);

	/* Begin Reading Input */
	while(get_line(is) >= 0) {
		if(is->NF>1){
			/* PERSON */
			if(strcmp(is->fields[0], "PERSON") == 0){
				// retrive name //
				name = createName(is);

				// check if person is in the tree already //	
				tmp = jrb_find_str(people, name);
				if(tmp == NULL){ // if person is not in the tree, then add person
					person = createPerson(name);  // create struct
					jrb_insert_str(people, name, new_jval_v((void *) person));  // insert into tree
					savedPerson = person;  // save current person
				}
				else{  // else just save the person for further use
					savedPerson = (Person*) tmp->val.v;
					free(name);
				}
			}
			/* FATHER */
			if(strcmp(is->fields[0], "FATHER") == 0){
				// retrieve name //
				name = createName(is);

				// check if person is in tree already //
				tmp = jrb_find_str(people, name);
				if(tmp == NULL){  // if not create the person
					person = createPerson(name);
					jrb_insert_str(people, name, new_jval_v((void *) person));  // insert into tree
				}
				else{  // if person already in tree, retrieve father pointer
					person = (Person*) tmp->val.v;  // person is the FATHER
					free(name);
				}

				/* father is person, child is savedPerson, realFather is child->father*/
				father = person;
				child = savedPerson;
				realFather = (Person*)(child->father);

				// check the sex of father
				if(checkSex(father, 'M') == 0){
					fprintf(stderr,"Bad input - sex mixmatch on line %i\n", is->line);
					exit (1);
				}

				// check the links of child and father //
				if(realFather!=NULL && strcmp(realFather->name, father->name)!=0){ 
					fprintf(stderr,"Bad input -- child with two fathers on line %i\n", is->line);
					exit(1);
				}

				// add child to father's list & link father and child
				addChild(child, father->children);
				child->father = father;
			}
			/* MOTHER */
			if(strcmp(is->fields[0], "MOTHER") == 0){	
				// retrieve name //
				name = createName(is);

				// check if person is already in tree //
				tmp = jrb_find_str(people, name);
				if(tmp == NULL){
					person = createPerson(name);
					jrb_insert_str(people, name, new_jval_v((void *) person));  // insert into tree
				}
				else{  // else retreive person for use
					person = (Person*) tmp->val.v;
					free(name);
				}

				/* mother is person, child is savedPerson, realMother is child->mother */
				mother = person;
				child = savedPerson;
				realMother = (Person*)child->mother;

				// check the sex of the mother
				if(checkSex(mother, 'F') == 0){
					fprintf(stderr,"Bad input - sex mismatch on line %i\n", is->line);		
					exit(1);
				}

				// check link between mother and child
				if(realMother!=NULL && strcmp(realMother->name, mother->name)!=0){
					fprintf(stderr,"Bad input -- child with two mothers on line %i\n", is->line);
					exit(1);
				}

				// add child to Mother's list & link mother and child
				addChild(child, mother->children);
				child->mother = mother;
			}
			/* FATHER_OF */
			if(strcmp(is->fields[0], "FATHER_OF") == 0){
				// retreive name //
				name = createName(is);

				// check if person is already in tree //
				tmp = jrb_find_str(people, name);
				if(tmp == NULL){
					person = createPerson(name);
					jrb_insert_str(people, name, new_jval_v((void *) person));  // insert into tree
				}
				else{
					person = (Person*)tmp->val.v;  // save person for use
					free(name);
				}

				/* child is person, father is savedPerson, realFather is child->father */
				child = person;
				father = savedPerson;
				realFather = (Person*) child->father;

				// check the sex of father
				if(checkSex(father, 'M') == 0){
					fprintf(stderr,"Bad input - sex mismatch on line %i\n", is->line);	
					exit(1);
				}

				// check links between child and father
				if(realFather!=NULL && strcmp(realFather->name, father->name)!=0){
					fprintf(stderr,"Bad input -- child with two fathers on line %i\n", is->line);
					exit(1);
				}

				// add child to father's list and link father and child
				addChild(child, father->children);
				child->father = father;
			}
			/* MOTHER_OF */
			if(strcmp(is->fields[0], "MOTHER_OF") == 0){
				// retrieve name //
				name = createName(is);

				// check if person is in tree or not //
				tmp = jrb_find_str(people, name);
				if(tmp == NULL){
					person = createPerson(name);
					jrb_insert_str(people, name, new_jval_v((void *) person));  // insert into tree		
				}
				else{
					person = (Person*) tmp->val.v;  // save person for use
					free(name);
				}

				/* child is person, mother is savedPerson, realMother is child->mother */
				child = person;
				mother = savedPerson;
				realMother = (Person*)child->mother;

				// check the sex of the mother
				if(checkSex(mother, 'F') == 0){
					fprintf(stderr,"Bad input - sex mismatch on line %i\n", is->line);		
					exit(1);
				}

				// check links between mother and child
				if(realMother!=NULL && strcmp(realMother->name, mother->name)!=0){
					fprintf(stderr,"Bad input -- child with two mothers on line %i\n", is->line);		
				}

				// add child to mother's list and link 
				addChild(child, mother->children);
				child->mother = mother;
			}
			/* SEX */
			if(strcmp(is->fields[0], "SEX") == 0){
				if(strcmp(is->fields[1], "F") == 0){   // FEMALE
					if(checkSex(savedPerson, 'F')==0){
						fprintf(stderr,"Bad input - sex mismatch on line %i\n", is->line);	
						exit(1);
					}
					savedPerson->sex = 'F';
				}
				else if(strcmp(is->fields[1], "M") == 0){   // MALE
					if(checkSex(savedPerson, 'M')==0){
						fprintf(stderr,"Bad input - sex mismatch on line %i\n", is->line);
						exit(1);
					}
					savedPerson->sex = 'M';
				}
			}
		}
	}

	// check for cycles
	jrb_traverse(tmp, people){
		person = (Person *) tmp->val.v;
		if(cycleCheck(person) ==1){
			fprintf(stderr, "Bad input -- cycle in specification\n");
			exit(1);
		}
	}

	//print the tree
	printTree(people);

	/* FREE MEMORY */
	jrb_traverse(tmp, people){
		person = (Person*) tmp->val.v;
		free_dllist(person->children);
		free(person->name);
		free(person);
	}

	jettison_inputstruct(is); //free up memory
	exit(0);
}
