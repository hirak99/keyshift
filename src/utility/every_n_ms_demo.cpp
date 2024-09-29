#include <iostream>

#include "every_n_ms.h"

int main() {
  for (int i = 0; i < 30; ++i) {
    // EVERY_N_MS(1000, std::cout << "Execution 1 at " << i << ", suppressed "
    //                            << suppressedCount << "\n\n");
    // EVERY_N_MS(1000, std::cout << "Execution 2 at " << i << "\n\n");
    EVERY_N_MS_W_SUPPRESSED(
        1000, std::cout << "EVERY_N_MS_W_SUPPRESSED 2 at " << i << "\n\n");
    CERR_EVERY_N_MS(1000, "CERR_EVERY_N_MS at " << i << "\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  return 0;
}
