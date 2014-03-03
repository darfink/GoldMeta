# Makefile for GoldMeta shared library

CC						= g++
FLAGS					= -std=c++11
CFLAGS				= -fPIC -pedantic -Wall -Wextra -ggdb3
LDFLAGS				= -shared
DEBUGFLAGS		= -O0 -D _DEBUG
RELEASEFLAGS	= -O2 -D NDEBEUG -combine -fwhole-program
NAME					= goldmeta

INC_IGNORE=halflife/dlls halflife/common halflife/engine halflife/public asmjit/src
INC_WARNING=udis86/libudis86 /usr/lib/boost/include include

# Add all of our include directories
FLAGS += $(foreach d, $(INC_IGNORE), -isystem $d)
FLAGS += $(foreach d, $(INC_WARNING), -I$d)

TARGET	= $(NAME).so
SOURCES = $(wildcard src/*.cpp) $(wildcard src/**/*.cpp)
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(FLAGS) $(CFLAGS) $(DEBUGFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

# vim: set ts=2 sw=2 noexpandtab: #
