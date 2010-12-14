CC=g++
CFLAGS=-c -g -Wall -Imodules/ -DDEBUG 
LDFLAGS=
DOCSCONFIG=Doxyfile
SOURCES=main.cpp $(filter-out $(SRC_EXCEPT), $(wildcard modules/*.cpp))
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=cc

.PHONY: docs clean graphs

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

docs:
	mkdir -p docs/
	doxygen $(DOCSCONFIG)

graphs:
	mkdir -p graphs/
	python graphgenerator.py cloud.log
	gnuplot *.plot
	mv *.plot *.data graphs/

clean:
	@rm -f $(EXECUTABLE)
	@rm -f *.o
	@rm -f modules/*.o
	@rm -f *~
	@rm -f modules/*~
	@rm -rf docs/html/
	@rm -rf graphs/
	@rm -f cloud.log


