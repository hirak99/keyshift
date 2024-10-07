#ifndef __INPUT_DEVICE_H
#define __INPUT_DEVICE_H

#include <fcntl.h>
#include <linux/input.h>

#include <chrono>
#include <thread>

const int kOpenRetryDurationMs = 2500;

class InputDevice {
 public:
  InputDevice(const char* device) {
    const auto start_time = std::chrono::steady_clock::now();
    while (true) {
      fd_ = open(device, O_RDONLY);
      // Return if succeeded.
      if (fd_ >= 0) return;
      // Not succeeded. Retry if within retry-duration.
      const auto elapsed = std::chrono::steady_clock::now() - start_time;
      if (elapsed >= std::chrono::milliseconds(kOpenRetryDurationMs)) {
        throw std::runtime_error("Error opening device");
      }
      // std::cerr << "Open failed. Retrying..." << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  }

  // Movable but not copyable.
  InputDevice(InputDevice&& other) = default;
  InputDevice& operator=(InputDevice&& other) = default;

  // Hides the device will from the operating system, so no other applications
  // process the events.
  void Grab() {
    if (grabbed_) return;

    // Wait until all keys are released. Otherwise the release event for any
    // already held key will get blocked once this device is grabbed.
    while (IsAnyKeyPressed()) {
      std::cerr << "Waiting for all keys to be released..." << std::endl;
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
