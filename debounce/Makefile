include ../Makefile.incl

.PHONY:	all clean clobber 

all:	debounce

debounce: debounce.o
	$(CXX) debounce.o -o debounce $(LDFLAGS)
	sudo chown root ./debounce
	sudo chmod u+s ./debounce

clean:
	rm -f *.o

clobber: clean
	rm -f debounce

