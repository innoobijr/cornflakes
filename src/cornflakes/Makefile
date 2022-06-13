IDIR = ./include
CC=gcc
ROOTIDIR=${HOME}/flexrpc/src/flexrpc/include
CFLAGS=-I$(ROOTIDIR)
CWD = ./

DEPS=cornflake.h

OBJ=cornflake.o
LBFILE=libcornflake.a

$(CWD)%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Default target

all: $(LBFILE)

$(LBFILE): $(OBJ)
	ar -cvq $(ROOTIDIR)/$@ $^
	ranlib $(ROOTIDIR)/$@

#flexxdr: $(OBJ)
#	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
	rm -f $(IDIR)/$(LBFILE) 
