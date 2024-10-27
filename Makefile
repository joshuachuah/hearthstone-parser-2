FLAG = -std=gnu11 -Werror -Wall

all: parser search 

parser:	parser.c 
	cc ${FLAG} -o parser parser.c -g 

search: search.c
	cc ${FLAG} -o search search.c -g