CC=gcc
CFLAGS= -I ./include/ -Wall -pedantic -std=c99
SRCS=rxfe.c dio_control.c
OBJS=rxfe.o dio_control.o
EXEC=xdio_control

user: $(EXEC)

$(EXEC) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

$(OBJS) : $(SRCS)
	$(CC) $(CFLAGS) -c $(SRCS)

clean:
	rm -f *~
	rm -f *#
	rm -f $(EXEC) $(OBJS)