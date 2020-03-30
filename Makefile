# compiler flags
CFLAGS := -O2 -flto

# object files
OBJ := src/main.o

.PHONY: all
all: lambda

lambda: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	find -name "*.o" -delete
	rm -f lambda

