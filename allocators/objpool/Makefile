CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -s -O2
TESTFLAGS = -Wall -Wextra -pedantic -std=c99 -g -O2

SRCS = objpool.c
OBJS = $(SRCS:.c=.o)

harbol_objpool:
	$(CC) $(CFLAGS) -c $(SRCS)

clean:
	$(RM) *.o
