
all: test
.PHONY: all

variants = debug release
compilers = g++ clang++
test-srcs = test_sum.cpp test_product.cpp

clean-files :=

test-release: variant-options = -O2
test-debug: variant-options =

test: $(tests)
.PHONY: test

define def-test-each-variant
test: test-$(variant)

test-$(variant): variant := $(variant)
.PHONY: test-$(variant)

$(foreach compiler,$(compilers),$(eval $(value def-test-each-compiler)))
endef

define def-test-each-compiler
test-$(variant): test-$(variant)-$(compiler)

test-$(variant)-$(compiler): CXX := $(compiler)
.PHONY: test-$(variant)-$(compiler)

$(foreach test-src,$(test-srcs),$(eval $(value def-test-each-src)))
endef

define def-test-each-src
test-$(variant)-$(compiler): test-$(variant)-$(compiler)-$(test-src)
test-$(variant)-$(compiler)-$(test-src): $(test-src)
	$(CXX) -std=c++20 $(CXXFLAGS) $(CPPFLAGS) $< -o prog-$@ -Werror -Wall -Wextra -pedantic-errors -g -I$$PWD/include
	./prog-$@
.PHONY: test-$(variant)-$(compiler)-$(test-src)

clean-files += prog-test-$(variant)-$(compiler)-$(test-src)
endef

$(foreach variant,$(variants),$(eval $(value def-test-each-variant)))

clean:
	$(RM) $(clean-files)
.PHONY: clean
