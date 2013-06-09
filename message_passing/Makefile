CPPFLAGS=-O0 -g -Wall -Wextra -pedantic -std=c++11
LDLIBS=-lstdc++ -lpthread

all: poll_based event_driven

poll_based: poll_based.o message.o
event_driven: event_driven.o message.o

clean:
	rm -f *~ *.o poll_based event_driven

.PHONY: all clean

