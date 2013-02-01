.PHONY: test

CC=gcc
CFLAGS= -Wall -pedantic

SRC1 = blink_test.c
OBJ1 = blink_test.o
EXE1 = xblink

SRC2 = cycle_test.c
OBJ2 = cycle_test.o
EXE2 = xcycle

SRC3 = test.c
OBJ3 = test.o
EXE3 = xtest


all: $(EXE1) $(EXE2) $(EXE3)
blink: $(EXE1)
cycle: $(EXE2)
test: $(EXE3)


$(EXE1) : $(OBJ1)
	$(CC) $(CFLAGS) $(OBJ1) -o $(EXE1)
$(OBJ1) : $(SRC1)
	$(CC) $(CFLAGS) -c $(SRC1)

$(EXE2) : $(OBJ2)
	$(CC) $(CFLAGS) $(OBJ2) -o $(EXE2)
$(OBJ2) : $(SRC2)
	$(CC) $(CFLAGS) -c $(SRC2)

$(EXE3) : $(OBJ3)
	$(CC) $(CFLAGS) $(OBJ3) -o $(EXE3)
$(OBJ3) : $(SRC3)
	$(CC) $(CFLAGS) -c $(SRC3)

clean:
	rm -f $(EXE1) $(OBJ1)
	rm -f $(EXE2) $(OBJ2)
	rm -f $(EXE3) $(OBJ3)
	rm -f *~
	rm -f *#