# Makefile
COMPILER = clang++
SOURCE_LIBS = -Ivendor/raylib/src
OSX_OPT = -Lvendor/raylib/src -lraylib -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL
CFLAGS = -std=c++17 -O1 -w

# Define source files
SRCS = src/main.cpp src/player.cpp src/world.cpp
OBJS = $(SRCS:.cpp=.o)

# Target executable
TARGET = mini_minecraft

all: raylib $(TARGET)

raylib:
	cd vendor/raylib/src && $(MAKE)

$(TARGET): $(SRCS)
	$(COMPILER) $(SRCS) -o $(TARGET) $(CFLAGS) $(SOURCE_LIBS) $(OSX_OPT)

clean:
	rm -f $(TARGET)
	cd vendor/raylib/src && $(MAKE) clean

.PHONY: all raylib clean
