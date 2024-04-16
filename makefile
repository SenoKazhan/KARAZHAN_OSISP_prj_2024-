CC = g++
CFLAGS = -std=c++11 -g2 -ggdb -pedantic -W -Wall -Wno-unused-parameter -Werror
GTK_INCLUDE = `pkg-config --cflags gtk+-3.0 gtkmm-3.0`
GTK_LIBS = `pkg-config --libs gtk+-3.0 gtkmm-3.0`
LDFLAGS = -lboost_filesystem -lncurses -lgtk-3 -lgtkmm-3.0 -lgobject-2.0 -lz -lpng -ljpeg
.SUFFIXES:
.SUFFIXES: .cpp .o
DEBUG = ./build/debug
RELEASE = ./build/release
OUT_DIR = $(DEBUG)
vpath %.cpp src
vpath %.h src
vpath %.o $(DEBUG)
objects = $(OUT_DIR)/FileManager.o $(OUT_DIR)/Panel.o $(OUT_DIR)/File.o $(OUT_DIR)/InputHandler.o $(OUT_DIR)/main.o
app = $(OUT_DIR)/app
all: $(app)
$(app): $(objects)
	$(CC) $(CFLAGS) $(objects) -o $@ $(GTK_LIBS) $(LDFLAGS)
$(OUT_DIR)/%.o: %.cpp %.h
	$(CC) -c $(CFLAGS) $(GTK_INCLUDE) $< -o $@

$(OUT_DIR)/main.o: main.cpp
	$(CC) -c $(CFLAGS) $(GTK_INCLUDE) $< -o $@

.PHONY: clean
clean:
	@rm -rf $(DEBUG)/* $(RELEASE)/* app

# 	$(CC) $(CFLAGS) $(objects) -o $@ $(GTK_LIBS) $(LDFLAGS) all в папке с makefile
# $(app): $(objects)
