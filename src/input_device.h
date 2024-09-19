#ifndef __INPUT_DEVICE_H
#define __INPUT_DEVICE_H

#include <fcntl.h>
#include <linux/input.h>

#include <chrono>
#include <thread>

class InputDevice {
 public:
  InputDevice(const char *device) {
    // Open the input device
    fd_ = open(device, O_RDONLY);
    if (fd_ < 0) {
      throw std::runtime_error("Error opening device");
    }
  }

  // Hides the device will from the operating system, so no other applications
  // process the events.
  void Grab() {
    if (grabbed_) return;

    // Wait until all keys are released. Otherwise the release event for any
    // already held key will get blocked once this device is grabbed.
    while (IsAnyKeyPressed()) {
      std::cerr << "Wating a sec, release all keys!" << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Grab the device
    if (ioctl(fd_, EVIOCGRAB, 1) < 0) {
      close(fd_);
      throw std::runtime_error("Error grabbing device");
    }
    grabbed_ = true;
  }

  ~InputDevice() {
    if (fd_ >= 0) {
      if (grabbed_) {
        // Release the device
        ioctl(fd_, EVIOCGRAB, 0);
      }
      close(fd_);
      std::cerr << "Closed the input device." << std::endl;
    }
  }

  int get_fd() const { return fd_; }

 private:
  bool IsAnyKeyPressed() {
    // KEY_CNT / 8 + 1 since one bit will be used per key.
    unsigned char key_state[KEY_CNT / 8 + 1];
    memset(key_state, 0, sizeof(key_state));

    if (ioctl(fd_, EVIOCGKEY(sizeof(key_state)), key_state) < 0) {
      perror("EVIOCGKEY");
      return -1;
    }

    for (std::size_t i = 0; i < sizeof(key_state); ++i) {
      if (key_state[i] != 0) {
        // Some key for this particular bit-field is pressed.
        return true;
      }
    }
    return false;
  }

  int fd_ = -1;
  bool grabbed_ = false;
};

#endif  // __INPUT_DEVICE_H
