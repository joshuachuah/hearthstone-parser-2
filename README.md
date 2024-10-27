[![Open in Codespaces](https://classroom.github.com/assets/launch-codespace-f4981d0f882b2a3f0472912d15f9806d57e124e0fc890972558857b51b24a6f9.svg)](https://classroom.github.com/open-in-codespaces?assignment_repo_id=10479519)
# A2: Hearthstone Card Search

Learning Outcomes
-----------------

1.  **Write** a Makefile to compile a simple C program.
2.  **Use** file stream functions to open and read files.
3.  **Use** file stream functions to read and write binary files.
4.  **Create** a program that will parse file content for display.
5.  **Create** a program that can search a database using an index data structure.
6.  **Use** memory allocator functions.

Specifications
--------------

For this assignment, you will continue building the program from A1 that parses a .csv file full of Hearthstone card data. It will also de-duplicate, sort and then **dump the cards to a pair of binary files**. These files will then be used by a second program to search for cards the user enters at the prompt.

### Stream I/O

You may use file streams for this assignment. Gone are the dark ages of having to deal with a too-tiny buffer!

### Array of Pointers to Structs

The data structure we want to use for this assignment is **a dynamic array of `CARD_T*`** or pointers to `CARD_T` structs. That means the data type of `cards` needs to be `CARD_T**`. You will also need a companion data structure **a dynamic array of `INDEX_T*`** or pointers to `INDEX_T` structs. This will be used to store a card's name and offset inside the `index.bin` file.

### Obtaining a Line from stdin

I strongly recommend continuing to use the `getline()` function. You can pass this function the special wizard word `stdin` in order for it to read from the standard input stream (this is where characters you type on the keyboard end up). You should read user input until the user enters a single lowercase `q` and presses enter. Don't forget to `free()` the memory allocated by the function before you exit the program or you will have a Valgrind error!

### Binary File Format

It's difficult to communicate visually on eLearning (or Mimir) the format of the binary files. Using the tool **xxd** we can obtain a hexdump of the two files that the solution program generates:

    $ xxd cards.bin | head
    00000000: 84fd 0000 0100 0000 0200 0000 0900 0000  ................
    00000010: 0400 0000 7400 0000 1b5b 316d 5175 6573  ....t....[1mQues
    00000020: 746c 696e 653a 1b5b 306d 2054 616b 6520  tline:.[0m Take
    00000030: 380a 6461 6d61 6765 206f 6e20 796f 7572  8.damage on your
    00000040: 2074 7572 6e73 2e0a 1b5b 316d 5265 7761   turns...[1mRewa
    00000050: 7264 3a1b 5b30 6d20 1b5b 316d 4c69 6665  rd:.[0m .[1mLife
    00000060: 7374 6561 6c1b 5b30 6d2e 2044 6561 6c20  steal.[0m. Deal
    00000070: 2433 0a64 616d 6167 6520 746f 2074 6865  $3.damage to the
    00000080: 2065 6e65 6d79 2068 6572 6f2e 0000 0000   enemy hero.....
    00000090: 0000 0000 3a0b 0000 0300 0000 0200 0000  ....:...........
    $ xxd index.bin | head
    00000000: 7e0d 0000 0000 0000 1700 0000 4120 4c69  ~...........A Li
    00000010: 6768 7420 696e 2074 6865 2044 6172 6b6e  ght in the Darkn
    00000020: 6573 7345 6104 0000 0000 0013 0000 0041  essEa..........A
    00000030: 204e 6577 2043 6861 6c6c 656e 6765 722e   New Challenger.
    00000040: 2e2e 8024 0100 0000 0000 1200 0000 4162  ...$..........Ab
    00000050: 6572 7261 6e74 2042 6572 7365 726b 6572  errant Berserker
    00000060: 0cea 0300 0000 0000 1100 0000 4162 6f6d  ............Abom
    00000070: 696e 6162 6c65 2042 6f77 6d61 6e50 0301  inable BowmanP..
    00000080: 0000 0000 0015 0000 0041 626f 6d69 6e61  .........Abomina
    00000090: 626c 6520 4c69 6575 7465 6e61 6e74 9e59  ble Lieutenant.Y
    
    

Notice that this is just the top of each file (the first 160 bytes). The `index.bin` file will first contain the `total_cards` value in **8 bytes**. Then it will contain that many records of:

1.  **4 bytes** for the _length of the card name_
2.  **?? bytes** for the _card name_ (the previous 4 bytes is how many bytes you need to read to obtain the entire card name)
3.  **8 bytes** for the _offset into the `cards.bin` file where the rest of the data lies_

The `cards.bin` file will contain `total_cards` number of records of:

1.  **4 bytes** for the _card id_
2.  **4 bytes** for the _card cost_
3.  **4 bytes** for the _card type_
4.  **4 bytes** for the _card class_
5.  **4 bytes** for the _card rarity_
6.  **4 bytes** for the _length of the card text_
7.  **?? bytes** for the _card text_ (the previous 4 bytes is how many you need bytes you need to read to obtain the entire card text)
8.  **4 bytes** for the _card's attack_
9.  **4 bytes** for the _card's health_

Now it might be clearer how much easier it is to use enums for data storage. For any of the `char*` fields, we must store both **the length** and **the data itself**. However, the rest of the fields are simply numbers. The only number that needs more than **4 bytes** is the offset for the card in the `index.bin` file.

### Restrictions

*   You must compile your program using the flags `std=gnu11`, `-Werror` and `-Wall`.
*   You must implement the stubbed functions: `dupe_check()`, `fix_text()`, `free_card()`, `parse_card()`, `print_card()`
*   You must adhere to the expected binary format or the testers will fail!

### Echoing Search String When Input Is Redirected

You must call `isatty(0)` to determine whether input is coming from a terminal or redirected file input. If you have an `input.txt` file with contents:

    BEEEES!!!
    Flamestrike
    Notareal Card
    Slep Bunwalla
    Goliath, Sneed's Masterpiece
    Leeroy Jenkins
    q

(which is the search strings for the example listed below) and you execute your program like:

    $ ./search < input.txt

and the output **is not identical to the example below, then you have not echoed the search string when input is redirected file input instead of from the standard input stream**. Read the manpage for `isatty()` and call it in your program such that the search string is echoed (printed) to the screen before the search is performed and the result is printed.

If done correctly, it will not matter if your program is executed as above or as below in the example, both should yield identical output.

### Submission

Submit only your completed `parser.c` file, `search.c` file, `card.h` and associated `Makefile`. When `make` is executed in the directory, a binary named `parser` **and also a binary named `search`** should be created. If not, the autograder will fail. You can make a submission as many times as you'd like in order to see the autograder feedback. The maximum points you can receive from the autograder is 12 and the remaining 12 points are from code review.

### Functions

Here is a list of functions that are used by the solution programs. This might help in completing the assignment:

*   `atoi()`
*   `bsearch()`
*   `build_index()`
*   `dupe_check()`
*   `fclose()`
*   `find_compar()`
*   `fix_text()`
*   `fopen()`
*   `fprintf()`
*   `fread()`
*   `free()`
*   `free_card()`
*   `free_index()`
*   `fseeko()`
*   `ftello()`
*   `fwrite()`
*   `getline()`
*   `isatty()`
*   `lfind()`
*   `malloc()`
*   `memcpy()`
*   `memmove()`
*   `parse_card()`
*   `print_card()`
*   `printf()`
*   `qsort()`
*   `read_card()`
*   `realloc()`
*   `search_compar()`
*   `sort_compar()`
*   `strcmp()`
*   `strdup()`
*   `strlen()`
*   `strsep()`
*   `strstr()`
*   `write_card()`
*   `write_index()`

Example
-------

Here's sample out from a working program. ***Your program must not print out anything other than what's shown or the autograder will fail***. The `$` character represents the terminal prompt, you do not type this character when executing commands.

    $ ./parser hscards.csv
    $ ./search
    >> BEEEES!!!
    BEEEES!!!                      3
    Spell        Druid        Common
    --------------------------------
    Choose a minion.
    Summon four 1/1 Bees
    that attack it.
    --------------------------------
    1                              4
    
    >> Flamestrike
    Flamestrike                    7
    Spell         Mage          Free
    --------------------------------
    Deal $5 damage to all enemy minions.
    --------------------------------
    0                              0
    
    >> Notareal Card
    ./search: 'Notareal Card' not found!
    >> Slep Bunwalla
    ./search: 'Slep Bunwalla' not found!
    >> Goliath, Sneed's Masterpiece
    Goliath, Sneed's Masterpiece   8
    Minion      Neutral    Legendary
    --------------------------------
    Battlecry: Fire five rockets
    at enemy minions that deal
    2 damage each. (You pick
    the targets!)
    --------------------------------
    8                              8
    
    >> Leeroy Jenkins
    Leeroy Jenkins                 5
    Minion      Neutral    Legendary
    --------------------------------
    Charge. Battlecry: Summon two 1/1 Whelps for your opponent.
    --------------------------------
    6                              2
    
    >> q

