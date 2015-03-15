CFLAGS = -Wall -Wextra -Wdeclaration-after-statement #-ansi -pedantic #-Wno-unused 

all: bwm

bwm: bwm.o

bwm.o: bwm.c

install:
	cp bwm $(HOME)/bin

#install 0644 ..man..

clean:
	@rm -f *.o bwm

.PHONY: clean install
