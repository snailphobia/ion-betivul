CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lasound

all: play

play: play.o polling.o utils.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o play
