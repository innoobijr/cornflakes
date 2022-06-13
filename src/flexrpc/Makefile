IDIR = ./include
CC=gcc
CFLAGS=-I$(IDIR)
ROOTIDIR=${HOME}/flexrpc/include

ODIR=./obj
LDIR=../lib
LBFILE=libflexrpc.a

LIBS=

_DEPS = flexrpc.h flexrpc_msg.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = flexxdr.o flexxdr_mem.o flexrpc_prot.o flexrpc_cmsg.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Default target
.PHONY : all
all :  $(LBFILE)

$(LBFILE): $(OBJ)
	ar -cvq $(ROOTIDIR)/$@ $^
	ranlib $(ROOTIDIR)/$@

#flexxdr: $(OBJ)
#	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
	rm -f $(IDIR)/$(LBFILE) 
