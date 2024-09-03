CXX = g++

CXXFLAGS = -Wall -Wextra -O2

all: build/demo_send_keys

test: build/remap_operator_test
	build/remap_operator_test

build/demo_send_keys: demo_send_keys.cpp
	@mkdir -p build/
	$(CXX) $(CXXFLAGS) demo_send_keys.cpp -o build/demo_send_keys

build/remap_operator_test: remap_operator_test.cpp remap_operator.h
	@mkdir -p build/
	$(CXX) $(CXXFLAGS) remap_operator_test.cpp -o build/remap_operator_test

