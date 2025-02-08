INCLUDE = ./include 
SRC = ./src
CC = cc

build: $(SRC)/*.c $(INCLUDE)
	$(CC) main.c -o ./co -ggdb2 -Wall -I$(INCLUDE) $(SRC)/*.c

run: build 
	./co main.co -l -p -c -o ./dist/file.c
