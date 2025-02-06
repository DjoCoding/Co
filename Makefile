INCLUDE = ./include 
SRC = ./src
CC = cc

build: $(SRC)/* $(INCLUDE)/*
	$(CC) main.c -o ./co -ggdb2 -Wall -I$(INCLUDE) $(SRC)/*.c

run: build 
	./co main.co -l -p -c -o file.c
