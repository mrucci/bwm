CFLAGS = -Wall -Wextra -Wdeclaration-after-statement #-ansi -pedantic #-Wno-unused 

all: bwm

install:
	cp bwm $(HOME)/bin

clean:
	@rm -f *.o bwm

.PHONY: clean install
