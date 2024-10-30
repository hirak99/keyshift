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

// Creates a virtual keyboard input device.

#include <errno.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>

class VirtualDevice {
 public:
  VirtualDevice() {
    const int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
      perror("Unable to open /dev/uinput");
      return;
    }

    // Set up the uinput device
    struct uinput_setup setup;
    memset(&setup, 0, sizeof(setup));
    setup.id.bustype = BUS_USB;
    // Randomly generated fixed vendor and product codes for virtual keyboard.
    setup.id.vendor = 0x549c;
    setup.id.product = 0xb248;
    setup.id.version = 1;
    strcpy(setup.name, "Virtual Keyboard");

    // Enable the necessary event types and keys
    if (ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0) {
      perror("UI_SET_EVBIT failed");
      close(fd);
      return;
    }

    // Enable the device to send all possible keys.
    // So far it looks like KEY_MICMUTE is the highest with value 248.
    for (int keycode = KEY_ESC; keycode <= 255; ++keycode) {
      if (ioctl(fd, UI_SET_KEYBIT, keycode) < 0) {
        std::cerr << "Error setting key bit for keycode " << keycode
                  << std::endl;
        close(fd);
        return;
      }
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

  ~VirtualDevice() {
    if (!IsOpen()) return;
    // Clean up and destroy the uinput device
    if (ioctl(file_descriptor_, UI_DEV_DESTROY) < 0) {
      perror("UI_DEV_DESTROY failed");
    }
    close(file_descriptor_);
  }

  // Movable but not copyable.
  VirtualDevice(VirtualDevice&& other) = default;
  VirtualDevice& operator=(VirtualDevice&& other) = default;

  inline int IsOpen() const { return file_descriptor_ >= 0; }

  void DoKeyEvent(unsigned int code, int value) const {
    SendEvent(EV_KEY, code, value);
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
