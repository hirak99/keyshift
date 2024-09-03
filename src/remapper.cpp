// Note that this can be dangerous if we grab the keyboard and make Ctrl+C
// impossible.
// So, to test, run this with `sudo timeout 20s ./<binary>`.
//
// WIP
// - Take the keyboard device from comamnd line
// - Integrate with remapper, read from config
// - Move preview mode to command line --preview
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <unistd.h>

#include <stdexcept>

#include "virtual_device.h"

const bool kPreviewOnly = true;

class InputDevice {
 public:
  InputDevice(const char* device) {
    // Open the input device
    fd = open(device, O_RDONLY);
    if (fd < 0) {
      throw std::runtime_error("Error opening device");
    }
  }

  // Hides the device will from the operating system, so no other applications
  // process the events.
  void Grab() {
    if (grabbed_) return;
    // Grab the device
    if (ioctl(fd, EVIOCGRAB, 1) < 0) {
      close(fd);
      throw std::runtime_error("Error grabbing device");
    }
    grabbed_ = true;
  }

  ~InputDevice() {
    if (fd >= 0) {
      if (grabbed_) {
        // Release the device
        ioctl(fd, EVIOCGRAB, 0);
      }
      close(fd);
    }
  }

  int get_fd() const { return fd; }

 private:
  int fd = -1;
  bool grabbed_ = false;
};

int main() {
  printf("Wating a sec, release all keys! ...\n");
  sleep(1);
  printf("Starting now...\n");

  InputDevice device(
      "/dev/input/by-id/usb-Drunkdeer_Drunkdeer_G65_US_RYMicro-event-kbd");
  if (!kPreviewOnly) device.Grab();

  VirtualDevice out_device;

  struct input_event ie;
  while (read(device.get_fd(), &ie, sizeof(struct input_event)) > 0) {
    if (ie.type == EV_KEY) {
      // printf("Key %i %s\n", ie.code, ie.value ? "pressed" : "released");

      if (!kPreviewOnly) {
        // Process and remap key events here
        if (ie.value) {
          out_device.KeyPress(ie.code);
        } else {
          out_device.KeyRelease(ie.code);
        }
      }
    }
  }
  return 0;
}
