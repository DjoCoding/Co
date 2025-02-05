INCLUDE = ./include 
SRC = ./src
CC = cc

main: $(SRC)/* $(INCLUDE)/*
	$(CC) main.c -o main -ggdb2 -Wall -I$(INCLUDE) $(SRC)/*.c