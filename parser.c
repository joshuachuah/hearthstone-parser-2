#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "card.h"

/*
 * I've left these definitions in from the
 * solution program. You don't have to
 * use them, but the `dupe_check()` function
 * unit test expects certain values to be
 * returned for certain situations!
 */
#define DUPE -1
#define NO_DUPE -2

/*
 * These are the special strings you need to
 * print in order for the text in the terminal
 * to be bold, italic or normal (end)
 */
#define BOLD "\e[1m"
#define ITALIC "\e[3m"
#define END "\e[0m"

/*
 * You will have to implement all of these functions
 * as they are specifically unit tested by Mimir
 */
int dupe_check(unsigned, char*);
char *fix_text(char*);
void free_card(CARD_T*);
CARD_T *parse_card(char*);
void print_card(CARD_T*);
int sort_comp(const void *a, const void *b);
int find_compar(const void *a, const void *b);
/*
 * We'll make these global again, to make
 * things a bit easier
 */
CARD_T **cards = NULL;
size_t total_cards = 0;


int main(int argc, char **argv) {
	// TODO: 1. Open the file
	//       2. Read lines from the file...
	//          a. for each line, `parse_card()`
    //          b. add the card to the array
	//       3. Sort the array
	//       4. Print and free the cards
	//       5. Clean up!
	char *infile = argv[1];
	FILE* file = fopen(infile, "r");
	char *line = NULL; // line pointer
	size_t n = 0; 


	cards = malloc(sizeof(CARD_T) * (total_cards + 1));

	int result = getline(&line, &n, file); // first read to get rid of the header 
	while ((result = getline(&line, &n, file)) != -1){
		CARD_T *card = parse_card(line);
		int dupe = dupe_check(card->id, card->name);
		cards = realloc(cards, sizeof(CARD_T) * (total_cards + 1));
		
		// check if the card is not a duplicate 
		if (dupe == NO_DUPE){
			// if not a duplicate, add to the cards array at position total_cards
			cards[total_cards] = card;
			// increment total_cards
			total_cards++;
		} 
		// if dupe is higher than current card 
		else if (dupe == DUPE){
			// free memory for duplicate card
			free_card(card);
		}
		// if the card is duplicate but its value is 
		// lower or equal to the current card in the array
		else{
			// retrieve old card at index dupe 
			// from cards array 
			CARD_T *old_card = cards[dupe];

			// free allocated memory for old_card
			free_card(old_card);

			// replace old_card in the cards array 
			// with the new card
			cards[dupe] = card;
		}	
		
	}
	
	// open bin files
	FILE *outfile = fopen("cards.bin", "wb");
	if (outfile == NULL){
		perror("failed to open cards.bin");
	}

	FILE *outfile_index = fopen("index.bin", "wb");
	if (outfile_index == NULL){
		perror("failed to open index.bin");
	}

	// allocate memory for index 
	INDEX_T **index = malloc(total_cards * sizeof(INDEX_T));

	
	for (int i = 0; i < total_cards; i++){
		
		off_t offset = ftello(outfile);

		// Add index entry for card
		index[i] = malloc(sizeof(INDEX_T));
		index[i]->name = strdup(cards[i]->name);
		index[i]->offset = offset;

		// id 
		fwrite(&cards[i]->id, sizeof(unsigned), 1 , outfile);

		// cost
		fwrite(&cards[i]->cost, sizeof(unsigned), 1 , outfile);

		// type 
		fwrite(&cards[i]->type, sizeof(Type), 1 , outfile);

		// class
		fwrite(&cards[i]->class, sizeof(Class), 1 , outfile);

		// rarity 
		fwrite(&cards[i]->rarity, sizeof(Rarity), 1 , outfile);

		// length of card text
		int text_length = strlen(cards[i]->text);
		fwrite(&text_length, sizeof(int), 1 , outfile);

		// card text 
		fwrite(cards[i]->text, sizeof(char), text_length , outfile);

		// attack
		fwrite(&cards[i]->attack, sizeof(unsigned), 1 , outfile);

		// health
		fwrite(&cards[i]->health, sizeof(unsigned), 1 , outfile);

		free_card(cards[i]);
	}

	// sort the index
	qsort(index, total_cards, sizeof(INDEX_T*), sort_comp);

	// write the total cards to the top of the index.bin file
	fwrite(&total_cards, sizeof(size_t), 1, outfile_index);

	for (int i = 0; i < total_cards; i++){
		unsigned name_length = strlen(index[i]->name);
		fwrite(&name_length, sizeof(unsigned), 1 , outfile_index);
		fwrite(index[i]->name, sizeof(char), name_length, outfile_index);
		fwrite(&index[i]->offset, sizeof(off_t), 1 , outfile_index);
	}

	// Free memory allocated for index
	for (int i = 0; i < total_cards; i++){
		free(index[i]);
	}

	free(cards);
	free(index);
	fclose(file);
	fclose(outfile);
	fclose(outfile_index);
	return 0;
}
// comparator function 

// comparator for lfind 
int find_compar(const void *a, const void *b) {
	char *a_ptr = (char*) a;
	CARD_T *b_ptr = (CARD_T*) b;
	return strcmp(a_ptr, b_ptr->name);
}

// comparator for qsort 
int sort_comp(const void *a, const void *b){
	INDEX_T *a_ptr = *((INDEX_T**) a);
	INDEX_T *b_ptr = *((INDEX_T**) b);
	return strcmp(a_ptr->name, b_ptr->name); 
}
/*
 * This function has to return 1 of 3 possible values:
 *     1. NO_DUPE (-2) if the `name` is not found
 *        in the `cards` array
 *     2. DUPE (-1) if the `name` _is_ found in
 *        the `cards` array and the `id` is greater
 *        than the found card's id (keep the lowest one)
 *     3. The last case is when the incoming card has
 *        a lower id than the one that's already in
 *        the array. When that happens, return the
 *        index of the card so it may be removed...
 */

int dupe_check(unsigned id, char *name) {
    for (int i = 0; i < total_cards; i++) {
        CARD_T *card = cards[i];
        if (strcmp(name, card->name) == 0) {
            if (id > card->id) {
                return DUPE;
            } else {
                return i;
            }
        }
    }
    return NO_DUPE;
}

/*
 * This function has to do _five_ things:
 *     1. replace every "" with "
 *     2. replace every \n with `\n`
 *     3. replace every </b> and </i> with END
 *     4. replace every <b> with BOLD
 *     5. replace every <i> with ITALIC
 *
 * The first three are not too bad, but 4 and 5
 * are difficult because you are replacing 3 chars
 * with 4! You _must_ `realloc()` the field to
 * be able to insert an additional character else
 * there is the potential for a memory error!
 */
char *fix_text(char *text) {
	// make a copy of text so we do not
	// change the original copy of text 
	char *new_text = strdup(text); 
	
	char *stringp = text; 
	char *pointer;

	// replace every "" with "
	pointer = strstr(stringp, "\"\"");
	while (pointer != NULL){
		int position = pointer - stringp; // to find the index of the substring
		new_text[position] = '\"';
		memmove(&new_text[position + 1], &new_text[position + 2], strlen(&new_text[position]) - 1);

		// update stringp to point to the same location as text 
		stringp = new_text;
		pointer = strstr(stringp, "\"\"");
	}

	// replace every \n with '\n'
	pointer = strstr(stringp, "\\n");
	while (pointer != NULL){
		int position = pointer - stringp;
		new_text[position] = '\n';
		memmove(&new_text[position + 1], &new_text[position + 2], strlen(&new_text[position])- 1);

		// update stringp to point to the same location as text 
		stringp = new_text;
		pointer = strstr(stringp, "\\n");
	}

	// replace </b> with END
	pointer = strstr(stringp, "</b>");
	while (pointer != NULL){
		memcpy(pointer, END, strlen(END));

		// Remove the remaining characters after END
		int position = (pointer - stringp) + strlen(END);

		// moves the characters that come after the original </b> tag to the position right after the END string.
		memmove(&new_text[position], &new_text[position + 4 - strlen(END)], strlen(&new_text[position + 4 - strlen(END)]) + 1);

		stringp = new_text;
		pointer = strstr(stringp, "</b>");
	}

	// replace </i>
	pointer = strstr(stringp, "</i>");
	while (pointer != NULL){
		memcpy(pointer, END, strlen(END));
		
		// Remove the remaining characters after END
		int position = (pointer - stringp) + strlen(END);

		// moves the characters that come after the original </b> tag to the position right after the END string.
		memmove(&new_text[position], &new_text[position + 4 - strlen(END)], strlen(&new_text[position + 4 - strlen(END)]) + 1);

		stringp = new_text;
		pointer = strstr(stringp, "</i>");
	}

	// replace <b>
	pointer = strstr(stringp, "<b>");
	while (pointer != NULL){
		int position = pointer - stringp;
		new_text = realloc(new_text, sizeof(char ) * (strlen(new_text) + sizeof(BOLD) + 1)); 

		// shift the characters to the right by one
		// to create an empty space where the "<b>" string was found
		memmove(&new_text[position + 3], &new_text[position + 2], strlen(&new_text[position]) + 1); 
		
		// copy the characters of the bold string
		// to the empty space 
		memcpy(&new_text[position], BOLD, 4);

		// update stringp to point to the same location as text 
		stringp = new_text; 
		pointer = strstr(stringp, "<b>");

	}

	// replace <i>
	pointer = strstr(stringp, "<i>");
	while (pointer != NULL){
		int position = pointer - stringp;
		new_text = realloc(new_text, sizeof(char ) * (strlen(new_text) + sizeof(ITALIC)  + 1)); 

		// shift the characters to the right by one
		// to create an empty space where the "<i>" string was found
		memmove(&new_text[position + 3], &new_text[position + 2], strlen(&new_text[position]) + 1);
		
		// copy the characters of the italic string
		// to the empty space 	
		memcpy(&new_text[position], ITALIC, 4);

		// update stringp to point to the same location as text 
		stringp = new_text;
		pointer = strstr(stringp, "<i>");
	}

	return new_text;                                            
}

/*
 * This short function simply frees both fields
 * and then the card itself
 */
void free_card(CARD_T *card) {
	// free the fields
	free(card->name);
	free(card->text);
	// free itself
	free(card);
}

/*
 * This is the tough one. There will be a lot of
 * logic in this function. Once you have the incoming
 * card's id and name, you should call `dupe_check()`
 * because if the card is a duplicate you have to
 * either abort parsing this one or remove the one
 * from the array so that this one can go at the end.
 *
 * To successfully parse the card text field, you
 * can either go through it (and remember not to stop
 * when you see two double-quotes "") or you can
 * parse backwards from the end of the line to locate
 * the _fifth_ comma.
 *
 * For the fields that are enum values, you have to
 * parse the field and then figure out which one of the
 * values it needs to be. Enums are just numbers!
 */
CARD_T *parse_card(char *line) {
	CARD_T *card = malloc(sizeof(CARD_T));
	char *stringp = line; // copy of line 

	// getting the id 
	char *token = strsep(&stringp, ",");
	card->id = atoi(token);

	// getting the name 
	stringp++;
	token = strsep(&stringp, "\"");
	card->name = strdup(token);

	// getting the cost 
	stringp++;
	token = strsep(&stringp, ",");
	card->cost = atoi(token);

	// get the text 

	// if text field is empty 
	if (strncmp(stringp, ",", strlen(",")) == 0){
		card->text = strdup("");
	}else{ // if text field is not empty 

		// allocate memory for text 
		char new_text[1024]; 

		// keep track of the current position in the new_text array 
		int position = 0; 
		stringp++;

		char *endp = strstr(stringp, "\""); 
		while((endp = strstr(stringp, "\""))){ 
			// if there is a terminating quote 
			if (strncmp(endp + 1, ",", strlen(",")) == 0){
				token = strsep(&stringp, "\"");
				strcpy(new_text+position, token);
				break;
			}else{ 
				// if it is not a terminating quote 
				token = strsep(&stringp, "\",");
				stringp ++; 
				strcpy(new_text+position, token);
			}
		}
		if (!endp){
			token = strsep(&stringp, "\"\"");
			strcpy(new_text+position, token);
		}
		card->text = fix_text(new_text);
	}

	// getting attack 
	stringp++;
	token = strsep(&stringp, ",");
	card->attack = atoi(token);

	// getting health 
	token = strsep(&stringp, ",");
	card->health= atoi(token);

	// getting the type 
	stringp++;
	token = strsep(&stringp, "\"");
	
	if (token[0] == 'H'){
		card->type = HERO; 
	}else if (token[0] == 'M'){
		card->type = MINION; 
	}else if (token[0] == 'S'){
		card-> type = SPELL; 
	}else if (token[0] == 'W'){
		card->type = WEAPON; 
	}

	// getting the class of the card 
	stringp++;
	stringp++;
	token = strsep(&stringp, "\"");

	if (strcmp(token, "DEMONHUNTER") == 0) card->class = DEMONHUNTER;
	if (strcmp(token, "DRUID") == 0) card->class = DRUID;
	if (strcmp(token, "HUNTER") == 0) card->class = HUNTER;
	if (strcmp(token, "MAGE") == 0) card->class = MAGE;
	if (strcmp(token, "NEUTRAL") == 0) card->class = NEUTRAL;
	if (strcmp(token, "PALADIN") == 0) card->class = PALADIN;
	if (strcmp(token, "PRIEST") == 0) card->class = PRIEST;
	if (strcmp(token, "ROGUE") == 0) card->class = ROGUE;
	if (strcmp(token, "SHAMAN") == 0) card->class = SHAMAN;
	if (strcmp(token, "WARLOCK") == 0) card->class = WARLOCK;
	if (strcmp(token, "WARRIOR") == 0) card->class = WARRIOR;
	
	// getting rarity 
	stringp++;
	stringp++;
	token = strsep(&stringp, "\"");

	if (token[0] == 'F'){
		card->rarity = FREE;
	}else if (token[0] == 'C'){
		card->rarity = COMMON;
	}else if (token[0] == 'E'){
		card->rarity = EPIC;
	}else if (token[0] == 'R'){
		card->rarity = RARE;
	}else if (token[0] == 'L'){
		card->rarity = LEGENDARY;
	}
	

	return card;
}

/*
 * Because getting the card class centered is such
 * a chore, you can have this function for free :)
 */
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
