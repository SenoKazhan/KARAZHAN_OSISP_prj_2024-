CC = gcc

NCURSES_CFLAGS = `pkg-config --cflags ncursesw`
NCURSES_LIBS =  `pkg-config --libs ncursesw`

LIBS += $(NCURSES_LIBS)
CFLAGS += $(NCURSES_CFLAGS)

SRCS = fm.c
OBJS = $(SRCS: .c = .o)
PROG = filemanag

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

install:
	install -Dm 755 $(PROG) $(BINDIR)/$(PROG)
	install -Dm 644 fm.1 $(MANDIR)/man1/fm.1

