// How to use -
// int main() {
//   auto mutex = GetMutex("my_app_" + ANY_DYNAMIC_STRING);
//   if (!mutex) {
//     exit(-1);
//   }
//   // Optionally if you want to clear the mutex early -
//   mutex->reset();
//   // ...
// }
//
// Note: This appears to just maintain a file -
// /dev/shm/sem.SEMAPHORE_NAME
//
#ifndef __OS_LEVEL_MUTEX
#define __OS_LEVEL_MUTEX

#include <semaphore.h>

#include <optional>
#include <string>

class OSMutex {
 public:
  // A hashed version of the name is used. Therefore, we have no constraints on
  // the name.
  OSMutex(const char* name);

  ~OSMutex();

  // Movable but not copyable.
  OSMutex(OSMutex&& other) = default;
  OSMutex& operator=(OSMutex&& other) = default;

 private:
  std::string hashed_name_;
  sem_t* sem_;
};

std::optional<OSMutex> AcquireOSMutex(std::string name);

#endif  // __OS_LEVEL_MUTEX