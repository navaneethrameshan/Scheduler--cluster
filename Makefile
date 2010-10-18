CC=g++
CFLAGS=-c -g -Wall -Imodules/
LDFLAGS=
SOURCES=main.cpp $(filter-out $(SRC_EXCEPT), $(wildcard modules/*.cpp))
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=cc

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	@rm -f cc
	@rm -f *.o
	@rm -f modules/*.o

