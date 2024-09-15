#include <fcntl.h>
#include <linux/input.h>

class InputDevice {
 public:
  InputDevice(const char *device) {
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
      perror("Closed the input device.");
    }
  }

  int get_fd() const { return fd; }

 private:
  int fd = -1;
  bool grabbed_ = false;
};
