#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "card.h"

int comparator(const void *a, const void *b);
void print_card(CARD_T*);
int find_card(char *name, FILE *infile_cards, CARD_T* card);
INDEX_T **read_index();

INDEX_T **card_index;
size_t total_cards;

int main(int argc, char **argv){
    
    // opening the files
    FILE *infile = fopen("cards.bin", "rb");
	if (infile == NULL){
		fprintf(stderr, "Could not open cards.bin file");
		return -2;
	} 

    FILE *infile_index = fopen("index.bin", "rb");
    if (infile_index == NULL){
		fprintf(stderr, "Could not open index.bin file");
		return -2;
	}

	// variables to store the card name and length 
	size_t n = 0;
	char *line = NULL;

	// read the card index from index.bin 
	card_index = read_index();
	if (card_index == NULL){
		fprintf(stderr, "Could not read card index from index.bin");
		return -2;
	}

	// main loop to read card names from user
	while(1){
		printf(">> ");
		ssize_t chars_read = getline(&line, &n, stdin);

		if (!isatty(0)){
			printf("%s\n", line);
		}
		// removing newline character from input
		if (chars_read > 0 && line[chars_read - 1] == '\n'){
			line[chars_read - 1] = '\0';
		}

		// exit the loop if user inputs "q"
		if (strcmp(line, "q") == 0){
			if(isatty(0)){
				break;
			}
		}

		// search for the card and print it if found
		CARD_T card;
		int result = find_card(line, infile, &card);
		if (result == 0){
			print_card(&card);
			free(card.text);
		}else if (result == -1){
			printf("./search: '%s' not found!\n", line);
		}
		
	}


	// free allocated memory and close files 
	free(line);

	for (int i = 0; i < total_cards; i++){
		free(card_index[i]->name);
		free(card_index[i]);
	}
	free(card_index);

	fclose(infile);

	return 0;
}

// function to read the card index from index.bin

INDEX_T **read_index(){
	FILE *infile_index = fopen("index.bin", "rb");
	if (infile_index == NULL){
		fprintf(stderr, "Could not open index.bin file");
		return NULL;
	}
	
	INDEX_T **index;
	// read total cards and allocate memory for the index
	fread(&total_cards, sizeof(size_t), 1 , infile_index);
	index = malloc(sizeof(INDEX_T*) * (total_cards + 1));

	// read the card entries 
	for (int i = 0; i < total_cards; i++){
		// allocate memory for struct 
		INDEX_T *index_ptr = malloc(sizeof(INDEX_T));

		// read length of card name from file
		unsigned name_length;
		fread(&name_length, sizeof(unsigned), 1, infile_index);


		// allocate memory for card name and
		// read it from file 
		char *name = malloc(sizeof(char) * (total_cards + 1));
		fread(name, sizeof(char), name_length, infile_index);
      
		// Read the offset from the file
		off_t offset; 
		fread(&offset, sizeof(off_t), 1, infile_index);


		// assign the card name and offset 
		// to the index entry 
		index_ptr->name = name;
		index_ptr->offset = offset;

		// Store the pointer to the INDEX_T structure in the 'index' array
		index[i] = index_ptr;
	}

	// close the file 
	fclose(infile_index);
	return index;
}

// function to find a card by name
int find_card(char *name, FILE *infile, CARD_T *card){
	
	// binary search on the card index
	INDEX_T **found_index = bsearch(name, card_index, total_cards, sizeof(INDEX_T *), comparator);
	if (found_index){

		// seek to the card's offset in the cards.bin file
		fseeko(infile, (*found_index)->offset, SEEK_SET);

		int text_length = 0;
		char *text;

		// read card's details and assign the fields accordingly 

		// id 
		fread(&card->id, sizeof(unsigned), 1 , infile);

		// cost 
		fread(&card->cost, sizeof(unsigned), 1 , infile);

		// type 
		fread(&card->type, sizeof(Type), 1 , infile);

		// class 
		fread(&card->class, sizeof(Class), 1 , infile);

		// rarity 
		fread(&card->rarity, sizeof(Rarity), 1 , infile);

		// text
		fread(&text_length, sizeof(int), 1 , infile);

		text = malloc(sizeof(char) * (text_length));
		fread(text, sizeof(char), text_length , infile);
		text[text_length] = '\0';

		// attack
		fread(&card->attack, sizeof(unsigned), 1 , infile);

		// health
		fread(&card->health, sizeof(unsigned), 1 , infile);

		// assign to the respective fields 
		// in the card_t struct 
		card->text = text;
		card->name = name;

		return 0;
	}
	return -1;

}

// comparator function for bsearch 
int comparator(const void *a, const void *b) {
	// pointer to the key being searched for 
    const char *key = a; 

	// casts b to a pointer to a pointer to an INDEX_T struct,
	// since b is a pointer to an element in the sorted array 
    const INDEX_T **index_ptr = (const INDEX_T**)b; 

    return strcmp(key, (*index_ptr)->name);
}

void print_card(CARD_T *card) {
	printf("%-29s %2d\n", card->name, card->cost);
	unsigned length = 15 - strlen(class_str[card->class]);
	unsigned remainder = length % 2;
	unsigned margins = length / 2;
	unsigned left = 0;
	unsigned right = 0;
	if (remainder) {
		left = margins + 2;
		right = margins - 1;
	} else {
		left = margins + 1;
		right = margins - 1;
	}
	printf("%-6s %*s%s%*s %9s\n", type_str[card->type], left, "", class_str[card->class], right, "", rarity_str[card->rarity]);
	printf("--------------------------------\n");
	printf("%s\n", card->text);
	printf("--------------------------------\n");
	printf("%-16d%16d\n\n", card->attack, card->health);
}