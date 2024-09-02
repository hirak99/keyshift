CXX = g++

CXXFLAGS = -Wall -Wextra -O2

all: build/demo_send_keys

build/demo_send_keys: src/demo_send_keys
	@mkdir -p build/
	$(CXX) $(CXXFLAGS) src/demo_send_keys.cpp -o build/demo_send_keys
