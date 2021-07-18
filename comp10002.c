/* COMP10002 Foundations of Algorithms, Semester 1, 2021
 * Assigment 2 Text Stemming and POS Tagging
 * Full Name: Shiping Song     
 * Student Number: 1182443
 * Date:20/05/2021           
 */

/****** Include libraries ******/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define BS_NOT_FOUND (-1)
#define BS_FOUND 0
#define WORDSTRLEN 22 // Max length of first word
#define NULL_BYTE 1 // Space to store NULL character at end of each string 
#define MAX_POS_TAG 5 // Max amount of POS tags
#define MAX_LETTERS 4 // Max letters per POS tag
#define MAX_SPACE 4 // Max number of spaces for POS tag
#define MAX_FORM 4 // Max number of forms
#define FORM_LENGTH 25 // Max length of forms
#define MAX_UNIQ_WORDS 100 // Max number of unique words
#define LINE1_LENGTH WORDSTRLEN + NULL_BYTE + 1 // Length of Line 1
#define LINE2_LENGTH MAX_POS_TAG * MAX_LETTERS + MAX_SPACE + NULL_BYTE + 1 // Length of Line 2
#define LINE3_LENGTH FORM_LENGTH * MAX_FORM + MAX_FORM + NULL_BYTE // Length of Line 3
#define LINE_LENGTH_STAR 10 + NULL_BYTE // Length of ********** Line
#define SENTENCE_WORD_LENGTH 25 // Max length for Word of sentence
    
typedef char word_t[LINE1_LENGTH]; // the first word after $
typedef char POS_t[LINE2_LENGTH];  // to store the characters of 2nd line
typedef char form_t[LINE3_LENGTH]; // store all forms on line 3
typedef struct node node_t;
typedef char *data_t;

/* create node struct */
struct node {
	data_t data; 
	node_t *next;
};

/* create a linked list struct type */
typedef struct {
	node_t *head;
	node_t *foot;
} list_t;

/* create the dictionary */
typedef struct {
    word_t line1;
    POS_t line2;
    form_t line3;
    
} dict_t;

/****** Function Prototypes ******/
int getword(char W[], int limit);
int compare(void *a, void *b);
int binary_search(dict_t *dict, int lo, int hi, char A[], int *locn);
int is_empty_list(list_t *list);
int sequential_search(dict_t* array_dict, char* word, int count, char *root, int* locn2);

list_t *make_empty_list(void);
list_t *insert_at_foot(list_t *list, data_t word);

void free_list(list_t *list);
void remove_EOF_trailing(dict_t* array_dict, int count);
void stage1(dict_t *array_dict);
void stage2(dict_t *array_dict,int count);
void stage3(list_t *list, dict_t *array_dict, int count);
void stage4(list_t *list, dict_t *array_dict, int count);
void read_dict(dict_t* array_dict, int* count);
void make_word_list(list_t* list);

int main(int argc, char *argv[]){
    
    /* declare variables */
    dict_t array_dict[MAX_UNIQ_WORDS];
	list_t *list = make_empty_list();
    int count;

    /* read the file and put into dictionary */
	read_dict(array_dict, &count);

    /* read in sentence and store in linked list */
	make_word_list(list);
    
    /* remove newline characters for line1 in dict */ 
    remove_EOF_trailing(array_dict, count);
    
    /* go through each stage */
    stage1(array_dict);
    stage2(array_dict, count);
    stage3(list, array_dict, count);
    stage4(list, array_dict, count);
    
    /* free the list and set list to NULL */
    free_list(list);
    list = NULL;
	return 0;
}

/* reference to https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/getword.c*/

/* store a single word in an array from the standard input, no longer than
limit characters. Argument array is limit+1 characters */
int
getword(char W[], int limit) {
	int c, len=0;
	/* first, skip over any non alphabetics */
	while ((c=getchar())!=EOF && !isalpha(c)) {
		/* do nothing more */
	}
	if (c==EOF) {
		return EOF;
	}
	/* ok, first character of next word has been found */
	W[len++] = c;
	while (len<limit && (c=getchar())!=EOF && isalpha(c)) {
		/* another character to be stored */
		W[len++] = c;
	}
	/* now close off the string */
	W[len] = '\0';
	return 0;
}

/* reference to https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/listops.c 
   for all list functions */
   
/* makes an empty list */
list_t
*make_empty_list(void) {
	list_t *list;
	list = (list_t*)malloc(sizeof(*list));

	assert(list!=NULL);
	list->head = list->foot = NULL;

	return list;
}

/* frees allocated memory from the list */
void
free_list(list_t *list) {
	node_t *curr, *prev;

	assert(list!=NULL);
	curr = list->head;

	while (curr) {
		prev = curr;
		curr = curr->next;
		free(prev);
	}
	free(list);
}

/* Implementing a queue (FIFO) structure */
list_t
*insert_at_foot(list_t *list, data_t word) { 
	node_t *new;
	new = (node_t*)malloc(sizeof(*new));
	assert(list!=NULL && new!=NULL);

	new->data = word;
	new->next = NULL;

	if (list->foot==NULL) {
		/* this is the first insertion into the list */
		list->head = list->foot = new;
	} else {
		list->foot->next = new;
		list->foot = new;
	}
	return list;
}


/* compare function for qsort to sort strings alphabetically */
int 
compare(void *a, void *b){
    /* a & b are pointers-to-char *, so you have to dereference to char* */
	char *word1 = *(char **)a;
	char *word2 = *(char **)b;
    return strcmp (word1, word2);
}


/* references https://people.eng.unimelb.edu.au
              /ammoffat/ppsaa/c/binarysearch.c*/
              
// binary search function over the first line of the dictionary 
int
binary_search(dict_t *dict, int lo, int hi, char A[], int *locn){
	int mid, outcome;
	if (lo>=hi) {
		return BS_NOT_FOUND;
	}
	mid = (lo+hi)/2;
	if ((outcome = strcmp(A, dict[mid].line1)) < 0) { 
		return binary_search(dict, lo, mid, A, locn);
	} else if (outcome > 0) {
		return binary_search(dict, mid+1, hi, A, locn);
	} else {
		*locn = mid;
		return BS_FOUND;
	}
}

/* stage 1 function */
void 
stage1(dict_t *array_dict){

	/* printing out the first word of the dictionary, POS tag 
    and form associated */
	printf("==========================Stage 1==========================\n");
	printf("Word 0: %s\n", array_dict[0].line1);
    printf("POS: %s", array_dict[0].line2);
    printf("Form: %s", array_dict[0].line3);	
    }

/* stage 2 function */
void 
stage2(dict_t *array_dict, int count){
	/* intialize sum */
	double sum = 0;

	/* for loop to count over line 3 and 
    find the total number of variation forms*/
    for(int i=0;i<count;i++){
    	int  j= 0;
    	while(array_dict[i].line3[j]!='\0'){
			/* check if character is digit */
            if(isdigit(array_dict[i].line3[j])){
                sum++;
            }
            j++;
        }
   }
	printf("==========================Stage 2==========================\n");
	printf("Number of words: %d\n", count);
    printf("Average number of variation forms per word: %.2lf\n", sum*1.0/count);
}

/* stage 3 function */
void 
stage3(list_t *list, dict_t* array_dict, int count){
    printf("==========================Stage 3==========================\n");
    	/* sort via the dictionary and *locn index in bsearch  */
    int locn;

	/* intialize new_word to start of linked list*/
    node_t *new_word = list->head;

	/*while not end of linked list */
	while (new_word!= NULL){

		/* run via all words in the dictionary */
		if (binary_search(array_dict, 0, count, new_word->data, &locn) 
        == BS_NOT_FOUND) {
			printf("%-26sNOT_FOUND\n", new_word->data);
		} else {
			/* if word is found print out word, and POS tags associated */
			printf("%-26s%s", new_word->data, array_dict[locn].line2);
		}
		/* point to the next word in linked list */
        new_word = new_word->next;
	}
}

/* stage 4 function */
void 
stage4(list_t* list, dict_t* array_dict,int count){
    printf("==========================Stage 4==========================\n");

	/* intialize new_word to start of linked list*/
    node_t *new_word = list->head;

	/* intialize location indexes and root word*/
    int locn, locn2;
    char root[FORM_LENGTH+NULL_BYTE];
    
	/* while not end of linked list */
	while (new_word!= NULL){
		
		/* binary search check if word is found in dict */
		if (binary_search(array_dict, 0, count, new_word->data, &locn) 
         == BS_FOUND) {
			printf("%-26s%-26s%s", new_word->data, new_word->data, 
            array_dict[locn].line2);

        /* if not found check word in list is in any of the forms 
		and get the root word if it is and index of root word */
		} else if(sequential_search(array_dict, new_word->data, 
        count, root, &locn2) == 1){
            printf("%-26s%-26s%s", new_word->data, root,array_dict[locn2].line2);
            
        } else {
			printf("%-26s%-26sNOT_FOUND\n", new_word->data, new_word->data);
            
		}
		/* point to the next word in linked list */
        new_word = new_word->next;
	}
}


/* removes newline character at the end of line1 */
void
remove_EOF_trailing(dict_t* array_dict, int count){
    for(int i=0;i<count;i++){
        array_dict[i].line1[strcspn((array_dict[i].line1), "\n")] = 0;
    }
}

/* searchs over the all the forms of line 3 and
if word is equal to one of the forms 
return 1 and point to index of root word and the root word itself */
int 
sequential_search(dict_t* array_dict, char* word, int count, char *root, 
                  int* locn2){
	
	/* iterate over the length of dict */
	for(int i=0;i<count;i++){
		/* intialize delimiter and pointer token */
		char delim[] = "0123";
		char* token;

		/* the first form of line3 */
	    for (token = strtok(array_dict[i].line3, delim); 
             token; token = strtok(NULL, delim)){
			/* remove EOF character from token if there is */
			token[strcspn(token, "\n")] = 0;
			/*if strings are equal get index of form, and get root word
            from form */
			if(strcmp(word, token)==0){
            	*locn2 = i;
            	strcpy(root, array_dict[i].line1);
            	return 1;
			}
		}
    }
    return 0;
}

/* reads the file into array_dict and returns the count of how many 
words there are*/
void read_dict(dict_t* array_dict, int* count){
	char skip[LINE_LENGTH_STAR];
	int count1 = 0;
    while(getchar() == '$'){
            fgets(array_dict[count1].line1, LINE1_LENGTH, stdin);
            fgets(array_dict[count1].line2, LINE2_LENGTH, stdin);
            fgets(array_dict[count1].line3, LINE3_LENGTH,stdin);
			
			/* remove the first character from line1 and line 3*/
            memmove(array_dict[count1].line3, array_dict[count1].line3 +1,
                    strlen(array_dict[count1].line3));
            count1++;    
			
    }
	*count = count1;
	/* skip the ********** line */
    fgets(skip,LINE_LENGTH_STAR ,stdin);   	
}

 /* get individual words of the sentence and store in linked list*/

void make_word_list(list_t* list){
    char* word;
    char sentence[MAX_UNIQ_WORDS];

    while(getword(sentence, SENTENCE_WORD_LENGTH+NULL_BYTE) != EOF){
		word = (char*)malloc(strlen(sentence) + NULL_BYTE);
		strcpy(word, sentence);
		insert_at_foot(list, word);
	}    
}

/* Worst Case time Complexity should be O(dmf)*/
/* "algorithms are awesome" */
