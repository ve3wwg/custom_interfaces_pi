include ../Makefile.incl

.PHONY:	all clean clobber 

all:	readadc readtemp writedac diode selten

diode:	diode.o
	$(CXX) diode.o -o ./diode $(LDFLAGS)

readadc: readadc.o
	$(CXX) readadc.o -o ./readadc $(LDFLAGS)

writedac: writedac.o
	$(CXX) writedac.o -o ./writedac $(LDFLAGS)

readtemp: readtemp.o
	$(CXX) readtemp.o -o ./readtemp $(LDFLAGS)

selten: selten.o
	$(CXX) selten.o -o ./selten $(LDFLAGS)

clean:
	rm -f *.o

clobber: clean
	rm -f readadc writedac readtemp diode selten

