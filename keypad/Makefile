include ../Makefile.incl

.PHONY:	all clean clobber 

all:	keypad combo

keypad: keypad.o
	$(CXX) keypad.o -o keypad $(LDFLAGS)

combo:	combo.o
	$(CXX) combo.o -o combo $(LDFLAGS)

clean:
	rm -f *.o

clobber: clean
	rm -f keypad combo

