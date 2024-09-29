#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

// Invocation examples -
//
// EVERY_N_MS(1000, std::cout << "This line will be printed every 1 sec\n");
//
// EVERY_N_MS(1000, std::cout << "Execution at " << i << ", suppressed "
//                            << suppressed_count << " count(s)\n");

#define EVERY_N_MS(ms, code)                                                 \
  do {                                                                       \
    static std::chrono::steady_clock::time_point lastExecution =             \
        std::chrono::steady_clock::now() - std::chrono::hours(10000);        \
    ;                                                                        \
    static std::mutex mtx;                                                   \
    static int suppressed_count = 0;                                         \
    std::lock_guard<std::mutex> lock(mtx);                                   \
    auto now = std::chrono::steady_clock::now();                             \
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now -          \
                                                              lastExecution) \
            .count() >= (ms)) {                                              \
      lastExecution = now;                                                   \
      code;                                                                  \
      suppressed_count = 0;                                                  \
    } else {                                                                 \
      ++suppressed_count;                                                    \
    }                                                                        \
  } while (0)

// Invocation example -
//
// CERR_EVERY_N_MS(1000, "Execution 2 at " << i << " count(s)\n");

#define CERR_EVERY_N_MS(ms, stream_for_stderr)                   \
  EVERY_N_MS(                                                    \
      ms,                                                        \
      if (suppressed_count > 0) {                                \
        std::cerr << "[" << suppressed_count << " suppressed] "; \
      } std::cerr                                                \
          << stream_for_stderr);
