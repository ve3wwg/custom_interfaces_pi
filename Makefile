include Makefile.incl

.PHONY:	all clean clobber 

SUBDIRS = hc165 hc595 mcp23017 pcf8574 pcf8591 debounce YL-040 keypad mpd

all:	
	for proj in $(SUBDIRS) ; do \
		$(MAKE) -$(MAKEFLAGS) -C ./$$proj all ; \
	done

clean:
	for proj in $(SUBDIRS) ; do \
		$(MAKE) -$(MAKEFLAGS) -C ./$$proj clean ; \
	done

clobber: clean
	for proj in $(SUBDIRS) ; do \
		$(MAKE) -$(MAKEFLAGS) -C ./$$proj clobber ; \
	done

