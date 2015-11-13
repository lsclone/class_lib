TARGET = ./lib/libclass_lib.so
CPPOBJS = $(patsubst %.cpp, %.o, $(wildcard ./*.cpp))

CC = gcc
CXX = g++

CXXFLAGS = -g -Wall -ggdb
CXXFLAGS += -I./
CXXLIBS = -lpthread -lm

all : $(TARGET)

$(TARGET) : $(CPPOBJS)
	$(CXX) -shared -fpic -o $@ $(CXXFLAGS) $+ $(CXXLIBS)

$(CPPOBJS) : %.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

distclean : clean
	rm -f $(TARGET)

clean :
	rm -f $(CPPOBJS)
