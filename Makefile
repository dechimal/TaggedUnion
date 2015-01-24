
.PHONY: test

test:
	$(CXX) -std=c++14 $(CXXFLAGS) $(CPPFLAGS) test.cpp -o test -Wall -Wextra
	./test
