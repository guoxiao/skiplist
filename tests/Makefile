CXXFLAGS=-std=c++11 -g -O2 -lpthread -Igoogletest/googletest/include -I.. -Igoogletest/googletest

all: skiplist
gtest-all.o: googletest/googletest/src/gtest-all.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

skiplist: ../skiplist.hpp *.cpp Makefile gtest-all.o
	$(CXX) $(CXXFLAGS)  -o skiplist_test *.cpp gtest-all.o
