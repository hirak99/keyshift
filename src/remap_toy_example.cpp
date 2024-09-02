// Note that this can be dangerous if it grabs the keyboard and makes Ctrl+C
// impossible. Always run this with `sudo timeout 20s ./<binary>`.
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <unistd.h>

#include <stdexcept>

#include "virtual_device.h"

class InputDevice {
 public:
  InputDevice(const char* device) {
    // Open the input device
    fd = open(device, O_RDONLY);
    if (fd < 0) {
      throw std::runtime_error("Error opening device");
    }

    // Grab the device
    if (ioctl(fd, EVIOCGRAB, 1) < 0) {
      close(fd);
      throw std::runtime_error("Error grabbing device");
    }
  }

  ~InputDevice() {
    if (fd >= 0) {
      // Release the device
      ioctl(fd, EVIOCGRAB, 0);
      close(fd);
    }
  }

  int get_fd() const { return fd; }

 private:
  int fd = -1;
};

int main() {
  printf("Wating a sec, release all keys! ...\n");
  sleep(1);
  printf("Starting now...\n");

  InputDevice device(
      "/dev/input/by-id/usb-Drunkdeer_Drunkdeer_G65_US_RYMicro-event-kbd");
  VirtualDevice out_device;

  struct input_event ie;
  while (read(device.get_fd(), &ie, sizeof(struct input_event)) > 0) {
    if (ie.type == EV_KEY) {
      // printf("Key %i %s\n", ie.code, ie.value ? "pressed" : "released");

      // Process and remap key events here
      if (ie.value) {
        out_device.KeyPress(ie.code);
      } else {
        out_device.KeyRelease(ie.code);
      }
    }
  }
  return 0;
}
