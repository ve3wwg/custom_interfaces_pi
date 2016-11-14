include ../Makefile.incl

.PHONY:	all clean clobber 

all:	librotctl.a yl040a yl040b yl040c fm1 yl040d fm2

yl040a: yl040a.o
	$(CXX) yl040a.o -o yl040a $(LDFLAGS)
	sudo chown root yl040a
	sudo chmod u+s yl040a

yl040b: librotctl.a yl040b.o
	$(CXX) yl040b.o -o yl040b -L. -lrotctl $(LDFLAGS)
	sudo chown root yl040b
	sudo chmod u+s yl040b

yl040c: librotctl.a yl040c.o
	$(CXX) yl040c.o -o yl040c -L. -lrotctl $(LDFLAGS)
	sudo chown root yl040c
	sudo chmod u+s yl040c

fm1:	librotctl.a fm1.o 
	$(CXX) fm1.o -L. -lrotctl -o fm1 $(LDFLAGS)
	sudo chown root fm1
	sudo chmod u+s fm1

yl040d: librotctl.a yl040d.o
	$(CXX) yl040d.o -o yl040d -L. -lrotctl $(LDFLAGS)
	sudo chown root yl040d
	sudo chmod u+s yl040d

fm2:	librotctl.a fm2.o 
	$(CXX) fm2.o -o fm2 -L. -lrotctl $(LDFLAGS)
	sudo chown root fm2
	sudo chmod u+s fm2

LOBJS	= switch.o rotary.o flywheel.o

librotctl.a: $(LOBJS)
	@rm -f librotctl.a
	$(AR) cr librotctl.a $(LOBJS)

clean:
	rm -f *.o

clobber: clean
	rm -f yl040a yl040b yl040c fm1 yl040d fm2 *.a
