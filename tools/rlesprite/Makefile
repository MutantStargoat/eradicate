src = $(wildcard src/*.c)
obj = $(src:.c=.o)
bin = rlesprite

CFLAGS = -pedantic -Wall -g
LDFLAGS = -lpng -lz

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
