#include "os_level_mutex.h"

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>

#include <cstring>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>

#include "../thirdparty/digestpp/digestpp.hpp"

// Custom exception for semaphore already exists
class SemaphoreExistsException : public std::runtime_error {
 public:
  explicit SemaphoreExistsException(const std::string& name)
      : std::runtime_error("Semaphore already exists: " + name) {}
};

std::string GenerateSHA1(const std::string& input) {
  return digestpp::sha1().absorb(input).hexdigest();
}

OSMutex::OSMutex(const char* name) : sem_(nullptr) {
  // Hash it since there are restrictions on the name, i.e. (1) must be <=251
  // bytes, and (2) must not contain '/'. See `man sem_overview`.
  hashed_name_ = "/" + GenerateSHA1(std::string(name));
  sem_ = sem_open(hashed_name_.c_str(), O_CREAT | O_EXCL, 0644, 1);
  if (sem_ == SEM_FAILED) {
    if (errno == EEXIST) {
      throw SemaphoreExistsException(hashed_name_);
    } else {
      throw std::runtime_error("Failed to create semaphore: " +
                               std::string(strerror(errno)));
    }
  }
}

OSMutex::~OSMutex() {
  if (sem_) {
    sem_close(sem_);
    sem_unlink(hashed_name_.c_str());
  }
}

std::optional<OSMutex> AcquireOSMutex(std::string name) {
  try {
    return OSMutex(name.c_str());
  } catch (SemaphoreExistsException&) {
    // Only catch the SemaphoreExistsException. (For anything else, crash.)
    return std::nullopt;
  }
}
