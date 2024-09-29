#include <iostream>

#include "every_n_ms.h"

int main() {
  for (int i = 0; i < 30; ++i) {
    EVERY_N_MS(1000, std::cout << "Execution 1 at " << i << " count(s)\n");
    EVERY_N_MS(1000, std::cout << "Execution 2 at " << i << " count(s)\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  return 0;
}
