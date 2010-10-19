CC=g++
CFLAGS=-c -g -Wall -Imodules/
LDFLAGS=
DOCSCONFIG=Doxyfile
SOURCES=main.cpp $(filter-out $(SRC_EXCEPT), $(wildcard modules/*.cpp))
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=cc

.PHONY: docs clean

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

docs:
	mkdir -p docs/
	doxygen $(DOCSCONFIG)

clean:
	@rm -f cc
	@rm -f *.o
	@rm -f modules/*.o
	@rm -f *~
	@rm -f modules/*~
	@rm -rf docs/html/


