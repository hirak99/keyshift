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
#include <termios.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>

#include "keycode_lookup.h"
#include "remap_operator.h"
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

// Disable echoing input when run in terminal.
void DisableEcho() {
  struct termios tty;
  if (tcgetattr(STDIN_FILENO, &tty) != 0) {
    perror("tcgetattr");
    return;
  }
  tty.c_lflag &= ~ECHO;  // Disable echo
  if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) != 0) {
    perror("tcsetattr");
  }
}

int main() {
  printf("Wating a sec, release all keys! ...\n");
  sleep(1);
  printf("Starting now...\n");
  DisableEcho();

  InputDevice device(
      "/dev/input/by-id/usb-Drunkdeer_Drunkdeer_G65_US_RYMicro-event-kbd");
  if (!kPreviewOnly) device.Grab();

  Remapper remapper;
  remapper.add_mapping("", KeyPressEvent(KEY_A),
                       {remapper.action_activate_mapping("fn_layer")});
  remapper.add_mapping("fn_layer", KeyPressEvent(KEY_S),
                       {KeyPressEvent(KEY_B)});
  remapper.add_mapping("fn_layer", KeyReleaseEvent(KEY_S),
                       {KeyReleaseEvent(KEY_B)});
  VirtualDevice out_device;

  if (kPreviewOnly) {
    auto echo_on_emit_fn = [](int key_code, int press) {
      std::cout << "  Out: " << (press ? "P " : "R ")
                << keyCodeToName(key_code) << std::endl;
    };
    remapper.SetCallback(echo_on_emit_fn);
  }

  struct input_event ie;
  while (read(device.get_fd(), &ie, sizeof(struct input_event)) > 0) {
    if (ie.type == EV_KEY) {
      // printf("Key %i %s\n", ie.code, ie.value ? "pressed" : "released");

      if (kPreviewOnly) {
        std::cout << "In: " << ie.value << " " << keyCodeToName(ie.code)
                  << std::endl;
      }

      remapper.process(ie.code, ie.value);

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
