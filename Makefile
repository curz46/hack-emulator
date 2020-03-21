SRC = $(wildcard src/*.c)
OBJ = $(patsubst src%,out%,$(SRC:.c=.o))
INCLUDE = -Isrc

bin/hack: $(OBJ)
	gcc $(CFLAGS) -o bin/hack $^

out/%.o: src/%.c
	gcc -g3 $(CFLAGS) $(CPPFLAGS) $(INCLUDE) -c -o $@ $<

.PHONY: install
install:
	cp bin/hack $(PREFIX)/bin
