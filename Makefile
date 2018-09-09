
.PHONY: all test test-all

all: test-all
test-all:
	@PS4= set -ex; for i in g++ clang++; do \
	  $(MAKE) test CXX=$$i; \
	done

test_srcs = test.cpp test_product.cpp

test:
	@PS4= set -ex; for i in $(test_srcs); do \
	  $(CXX) -std=c++14 $(CXXFLAGS) $(CPPFLAGS) test.cpp -o test -Wall -Wextra -pedantic -g -I$$PWD/include; \
	  ./test; \
	done
