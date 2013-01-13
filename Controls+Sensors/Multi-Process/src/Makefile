VERSION=1
REVISION=0

CPP=g++
IDIR=../include

_DEPS=ControlledSharedMemory.h
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))

ODIR=obj
_OBJ=ControlledSharedMemory.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

LIB=lib-ctrl-shared-memory.so
LIBDIR=../lib
LIBFLAGS= -shared -Wl,-soname,$(LIB).$(VERSION)
LIBNAME=$(LIB).$(VERSION).$(REVISION)

CFLAGS=-I$(IDIR) -fPIC -Wall -g

TESTLIBS= -l-ctrl-shared-memory -lpthread -lrt
TESTINCS=-Itest

all: lib
lib: $(LIB) 

test: test/Test.cpp
	$(CPP) -L$(LIBDIR) -o test/$@ $^ $(OBJ) $(TESTINCS) $(CFLAGS) $(TESTLIBS)

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CPP) -c -o $@ $< $(CFLAGS)

$(LIB): $(OBJ)
	$(CPP) $(LIBFLAGS) -o $(LIBDIR)/$(LIBNAME) $^ $(CFLAGS)
	ln -sf $(LIBDIR)/$(LIBNAME) $(LIBDIR)/$(LIB).$(VERSION)
	ln -sf $(LIBDIR)/$(LIBNAME) $(LIBDIR)/$(LIB)

.PHONY: clean test

clean:
	rm -f $(ODIR)/*.o *~ $(INCDIR)/*~ $(LIBDIR)/$(LIBNAME) test/test
