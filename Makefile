CFLAGS = -g3 -Wall -Wextra -Wconversion -Wcast-qual -Wcast-align -g
CFLAGS += -Winline -Wfloat-equal -Wnested-externs
CFLAGS += -pedantic -std=gnu99 -Werror

PROMPT = -DPROMPT

.PHONY: all clean

all: 33sh 33noprompt

33sh: sh.c
	gcc $(CFLAGS) $(PROMPT) $^ -o $@
33noprompt: sh.c
	gcc $(CFLAGS) $^ -o $@
clean:
	rm -f 33sh 33noprompt
