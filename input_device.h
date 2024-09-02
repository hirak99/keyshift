// Class to create a new keyboard input device.
//

#include <errno.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

class UinputDevice {
 public:
  UinputDevice() {
    const int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
      perror("Unable to open /dev/uinput");
      return;
    }

    // Set up the uinput device
    struct uinput_setup setup;
    memset(&setup, 0, sizeof(setup));
    setup.id.bustype = BUS_USB;
    setup.id.vendor = 0x1234;
    setup.id.product = 0x5678;
    setup.id.version = 1;
    strcpy(setup.name, "Remapped Virtual Keyboard");

    // Enable the necessary event types and keys
    if (ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0) {
      perror("UI_SET_EVBIT failed");
      close(fd);
      return;
    }
    if (ioctl(fd, UI_SET_KEYBIT, KEY_A) < 0) {
      perror("UI_SET_KEYBIT failed");
      close(fd);
      return;
    }

    if (ioctl(fd, UI_DEV_SETUP, &setup) < 0) {
      perror("UI_DEV_SETUP failed");
      close(fd);
      return;
    }

    if (ioctl(fd, UI_DEV_CREATE) < 0) {
      perror("UI_DEV_CREATE failed");
      close(fd);
      return;
    }

    file_descriptor_ = fd;
  }

  ~UinputDevice() {
    if (!IsOpen()) return;
    // Clean up and destroy the uinput device
    if (ioctl(file_descriptor_, UI_DEV_DESTROY) < 0) {
      perror("UI_DEV_DESTROY failed");
    }
    close(file_descriptor_);
  }

  inline int IsOpen() const { return file_descriptor_ >= 0; }

  void KeyPress(unsigned int code) const {
    SendEvent(EV_KEY, code, 1);
    SendEvent(EV_SYN, SYN_REPORT, 0);  // Synchronize
  }
  void KeyRelease(unsigned int code) const {
    SendEvent(EV_KEY, code, 0);
    SendEvent(EV_SYN, SYN_REPORT, 0);  // Synchronize
  }

 private:
  void SendEvent(unsigned int type, unsigned int code, int value) const {
    if (!IsOpen()) return;
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = type;
    ev.code = code;
    ev.value = value;

    if (write(file_descriptor_, &ev, sizeof(ev)) < 0) {
      perror("write failed");
    }
  }

  // If negative, then the file isn't opened and there was some error.
  int file_descriptor_ = -1;
};
