CC = gcc

NCURSES_CFLAGS = `pkg-config --cflags ncursesw panel`
NCURSES_LIBS =  `pkg-config --libs ncursesw panel`

LIBS += $(NCURSES_LIBS)
CFLAGS += $(NCURSES_CFLAGS)

SRCS = main.c
OBJS = $(SRCS: .c = .o)
PROG = main

prefix = usr
bindir = $(prefix)/bin
mandir = $(prefix)/share/man

BINDIR = $(DESTDIR)/$(bindir)
MANDIR = $(DESTDIR)/$(mandir)

all: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(PROG) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm *.o
	rm *~


