TARGET = ./socket_server
CPPOBJS = $(patsubst %.cpp, %.o, $(wildcard ./*.cpp))

CC = gcc
CXX = g++

CXXFLAGS = -g -Wall -ggdb
CXXFLAGS += -I../class_lib

CXXLIBS = -lpthread -lm
CXXLIBS += -L../class_lib/lib -lclass_lib

all : $(TARGET)

$(TARGET) : $(CPPOBJS)
	$(CXX) -o $@ $(CXXFLAGS) $+ $(CXXLIBS)

$(CPPOBJS) : %.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

distclean : clean
	rm -f $(TARGET)

clean :
	rm -f $(CPPOBJS)
