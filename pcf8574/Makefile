include ../Makefile.incl

.PHONY:	all clean clobber 

all:	i2cio lcd

i2cio: i2cio.o
	$(CXX) i2cio.o -o i2cio $(LDFLAGS)

lcd: lcd.o lcd1602.o
	$(CXX) lcd.o lcd1602.o -o lcd $(LDFLAGS)

lcd1602.o: lcd1602.hpp

clean:
	rm -f *.o

clobber: clean
	rm -f i2cio lcd

