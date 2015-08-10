
.PHONY: all test test-all

all: test-all
test-all:
	for i in g++ clang++; do \
	  $(MAKE) test CXX=$$i; \
	done \

test:
	$(CXX) -std=c++14 $(CXXFLAGS) $(CPPFLAGS) test.cpp -o test -Wall -Wextra -pedantic -g
	./test
