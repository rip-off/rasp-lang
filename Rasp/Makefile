# Declaration of variables
CC = clang++
CC_FLAGS = -std=c++11 -Wall -Werror 

# File names
EXEC = rasp
SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

all: $(EXEC)

clean:
	rm -f $(EXEC) $(OBJECTS)

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC)

%.o: %.cpp
	$(CC) -c $(CC_FLAGS) $< -o $@

DEPS := $(OBJECTS:.o=.d)

-include $(DEPS)

