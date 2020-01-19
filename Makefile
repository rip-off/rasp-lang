CC = clang++
CC_FLAGS = -std=c++11 -Wall -Werror -g

EXEC = rasp
SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(subst src/,obj/, $(subst .cpp,.o, $(SOURCES)))

all: test

clean:
	rm -f $(EXEC) $(OBJECTS)
	rmdir obj/

test: $(EXEC)
	./$(EXEC) --unit-tests

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC)

obj/%.o: src/%.cpp
	@mkdir -p obj/
	$(CC) -c $(CC_FLAGS) $< -o $@

DEPS := $(OBJECTS:.o=.d)

-include $(DEPS)

