include ../Makefile.incl

.PHONY:	all clean clobber 

all:	mcp_out mcp_in mcp_int

mcp_out: mcp_out.o
	$(CXX) mcp_out.o -o ./mcp_out $(LDFLAGS)
	sudo chown root ./mcp_out
	sudo chmod u+s ./mcp_out

mcp_in: mcp_in.o
	$(CXX) mcp_in.o -o ./mcp_in $(LDFLAGS)
	sudo chown root ./mcp_in
	sudo chmod u+s ./mcp_in

mcp_int: mcp_int.o
	$(CXX) mcp_int.o -o ./mcp_int $(LDFLAGS)
	sudo chown root ./mcp_int
	sudo chmod u+s ./mcp_int

clean:
	rm -f *.o

clobber: clean
	rm -f mcp_out mcp_in mcp_int

