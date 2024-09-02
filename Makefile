CXX = g++

CXXFLAGS = -Wall -Wextra -O2

all: build/demo_send_keys

test: build/remap_operator_test
	build/remap_operator_test

build/demo_send_keys: src/demo_send_keys.cpp
	@mkdir -p build/
	$(CXX) $(CXXFLAGS) src/demo_send_keys.cpp -o build/demo_send_keys

build/remap_operator_test: src/remap_operator_test.cpp src/remap_operator.h
	@mkdir -p build/
	$(CXX) $(CXXFLAGS) src/remap_operator_test.cpp -o build/remap_operator_test

