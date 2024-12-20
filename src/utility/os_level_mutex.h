/*
 * Copyright (c) 2024 Nomen Aliud (aka Arnab Bose)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// How to use -
// int main() {
//   auto mutex = AcquireOSMutex("my_app_" + ANY_DYNAMIC_STRING);
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