include ../Makefile.incl

.PHONY:	all clean clobber 

all:	hc595 hc165_595

hc595:	hc595.o
	$(CXX) hc595.o -o ./hc595 $(LDFLAGS)
	sudo chown root ./hc595
	sudo chmod u+s ./hc595

hc165_595: hc165_595.o
	$(CXX) hc165_595.o -o ./hc165_595 $(LDFLAGS)
	sudo chown root ./hc165_595
	sudo chmod u+s ./hc165_595

clean:
	rm -f *.o

clobber: clean
	rm -f hc595 hc165_595

