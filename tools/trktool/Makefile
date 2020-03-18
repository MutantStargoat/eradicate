src = $(wildcard src/*.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)
bin = trktool

inc = -I../../libs/cgmath/src -I../../src

CFLAGS = -pedantic -Wall -g -MMD $(inc)
LDFLAGS = -lGL -lGLU -lglut -lm

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)
