#include <chrono>
#include <mutex>
#include <thread>

#define EVERY_N_MS(ms, code)                                                 \
  do {                                                                       \
    static std::chrono::steady_clock::time_point lastExecution =             \
        std::chrono::steady_clock::now();                                    \
    static std::mutex mtx;                                                   \
    std::lock_guard<std::mutex> lock(mtx);                                   \
    auto now = std::chrono::steady_clock::now();                             \
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now -          \
                                                              lastExecution) \
            .count() >= (ms)) {                                              \
      lastExecution = now;                                                   \
      code;                                                                  \
    }                                                                        \
  } while (0)
