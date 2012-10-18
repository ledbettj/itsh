CC      = gcc

TARGET = itsh
SRCS   = main.c shell.c parser.c builtin.c

INCLUDEDIRS =
LIBDIRS     =
LIBS        = -lreadline

CFLAGS  = -Wall -ggdb3 -std=gnu99 $(INCLUDEDIRS)
LDFLAGS = $(LIBS) $(LIBDIRS)
OBJS   = $(SRCS:.c=.o)
DEPS   = $(SRCS:.c=.depends)


.PHONY: clean all check-syntax

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $(TARGET)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

%.depends: %.c
	$(CC) -M $(CFLAGS) $< > $@

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

check-syntax:
	$(CC) $(CFLAGS) -fsyntax-only $(CHK_SOURCES)

ifneq "$(MAKECMDGOALS)" "clean"
ifneq "$(MAKECMDGOALS)" "check-syntax"
-include $(DEPS)
endif
endif
