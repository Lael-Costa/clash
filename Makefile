CFLAGS = -g3 -Wall -Wextra -Wconversion -Wcast-qual -Wcast-align -g
CFLAGS += -Winline -Wfloat-equal -Wnested-externs
CFLAGS += -pedantic -std=gnu99 # -Werror
CC = gcc
all: clash

clash: clash.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f clash
