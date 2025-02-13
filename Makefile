INCLUDE = ./include 
SRC = ./src
CC = cc
DIST = ./dist

build: $(SRC)/*.c $(INCLUDE)
	$(CC) main.c -o ./co -ggdb2 -Wall -I$(INCLUDE) $(SRC)/*.c

debug: build
	gdb ./co

run: build 
	./co main.co -l -p -c -o ./dist/file.c

buildc: run
	$(CC) $(DIST)/file.c -I$(DIST) -o $(DIST)/file -ggdb2 

runc: buildc
	$(DIST)/file

debugc: buildc 
	gdb $(DIST)/file