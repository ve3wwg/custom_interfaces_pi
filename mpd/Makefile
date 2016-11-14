include ../Makefile.incl

.PHONY:	all clean clobber 

all:	mpcctl

mpcctl:	mpcctl.o ../pcf8574/lcd1602.o
	$(CXX) mpcctl.o ../pcf8574/lcd1602.o -o mpcctl $(LDFLAGS) -L$(TOPDIR)/YL-040 -lrotctl
	sudo chown root mpcctl
	sudo chmod u+s mpcctl

lcd1602.o:
	$(MAKE) -$(MAKEFLAGS) -C $(TOPDIR)/pcf8574/lcd1602.o

clean:
	rm -f *.o

clobber: clean
	rm -f mpcctl

