SOURCES = main.cpp safecall.cpp
CPPC = g++
CPPFLAGS = -c -Wall -O2 -std=c++11
LDLIBS = 
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = safecall

all: $(TARGET)

safecall.o: safecall.h
main.o: safecall.h
%.o : %.c
$(OBJECTS): Makefile 

.cpp.o:
	$(CPPC) $(CPPFLAGS) $< -o $@

$(TARGET): $(OBJECTS)
	$(CPPC) -o $@ $(OBJECTS) $(LDLIBS)

.PHONY: clean
clean:
	rm -f *~ *.o $(TARGET) 

