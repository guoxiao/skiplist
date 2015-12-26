CXXFLAGS=-std=c++11 -g -O2 -Wall -Wextra -Igoogletest/googletest/include -I.. -Igoogletest/googletest -DNDEBUG
LIBS=-lpthread

all: skiplist_test

gtest-all.o: googletest/googletest/src/gtest-all.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

skiplist_test: ../skiplist.hpp *.cpp Makefile gtest-all.o
	$(CXX) $(CXXFLAGS) $(LIBS) -o skiplist_test *.cpp gtest-all.o
