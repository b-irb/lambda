# compiler flags
CFLAGS := -O2 -flto
CDBGFLAGS = -fsanitize=address -ggdb

# object files
OBJ := src/main.o

.PHONY: all
all: lambda

lambda: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: debug
debug: lambda-debug

lambda-debug: $(OBJ)
	$(CC) $(CDBGFLAGS) $(OBJ) -o $@

%.o: %.c
	$(CC) $(CDBGFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	find -name "*.o" -delete
	rm -f lambda*

