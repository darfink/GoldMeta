# Makefile for GoldMeta shared library

CC						= g++
FLAGS					= -std=c++11 -Iinclude
CFLAGS				= -fPIC -pedantic -Wall -Wextra -ggdb3
LDFLAGS				= -shared
DEBUGFLAGS		= -O0 -D _DEBUG
RELEASEFLAGS	= -O2 -D NDEBEUG -combine -fwhole-program
NAME					= goldmeta

TARGET	= lib$(NAME).so
SOURCES = $(wildcard *.cpp) $(wildcard **/*.cpp)
HEADERS = $()
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(FLAGS) $(CFLAGS) $(DEBUGFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)
