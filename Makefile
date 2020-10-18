CC = g++
CC_FLAGS = -std=c++11 -Wall -Werror -g

EXEC = rasp
SOURCES = $(wildcard src/*.cpp)
OBJECT_DIR = obj/
OBJECTS = $(subst src/,$(OBJECT_DIR), $(subst .cpp,.o, $(SOURCES)))

all: test

clean:
	rm -f $(EXEC) $(OBJECTS)
	@if [ -d $(OBJECT_DIR) ]; then rmdir $(OBJECT_DIR); fi

test: $(EXEC)
	./$(EXEC) --unit-tests

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC)

obj/%.o: src/%.cpp
	@mkdir -p $(OBJECT_DIR)
	$(CC) -c $(CC_FLAGS) $< -o $@

DEPS := $(OBJECTS:.o=.d)

-include $(DEPS)

