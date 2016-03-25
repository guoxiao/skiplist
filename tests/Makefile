CXXFLAGS += -std=c++11 -g -O2\
			-Wall -Wextra -Weffc++\
			-pedantic\
			-fno-omit-frame-pointer\
			-isystem googletest/googletest/include\
			-I.. -isystem googletest/googletest\
			--coverage
LIBS=-lpthread

all: skiplist_test

gtest-all.o: googletest/googletest/src/gtest-all.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

skiplist_test: ../skiplist.hpp *.cpp Makefile gtest-all.o
	$(CXX) $(CXXFLAGS) *.cpp gtest-all.o $(LIBS) -o skiplist_test

test: skiplist_test
	./skiplist_test

cov: test
	lcov --capture --directory . --output-file skiplist_test.info --test-name skiplist_test
	genhtml skiplist_test.info --output-directory html
	xdg-open html/index.html

clean:
	rm -rf skiplist_test *.o *.gcda *.info html

.PHONY: test cov clean
