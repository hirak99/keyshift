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

// Appears to work without sudo.
// To test, run the compiled binary, and switch to some other place where a key
// in can be registered.
//
// Compile with: g++

#include "virtual_device.h"

int main() {
  VirtualDevice device;
  if (!device.IsOpen()) {
    return 1;
  }

  // Wait for a few secs after creating for tests.
  printf("Waiting a few secs...\n");
  sleep(2);

  // Send a key press event for KEY_B.
  device.DoKeyEvent(KEY_B, 1);
  sleep(1);  // Wait for a second.

  // Send a key release event for KEY_B.
  device.DoKeyEvent(KEY_B, 0);
  sleep(1);  // Wait for a second.

  return 0;
}
