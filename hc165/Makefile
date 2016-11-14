include ../Makefile.incl

.PHONY:	all clean clobber 

all:	hc165.o
	$(CXX) hc165.o -o ./hc165 $(LDFLAGS)
	sudo chown root ./hc165
	sudo chmod u+s ./hc165

clean:
	rm -f *.o

clobber: clean
	rm -f hc165

