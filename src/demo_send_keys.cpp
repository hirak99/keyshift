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
